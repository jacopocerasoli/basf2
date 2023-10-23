import itertools
from pathlib import Path
import numpy as np
import torch
from .tree_utils import masses_to_classes
from .dataset_utils import populate_avail_samples, preload_root_data
from .edge_features import compute_edge_features
from .normalize_features import normalize_features
from torch_geometric.data import Data, InMemoryDataset, Dataset
import uproot


def _preload(self):
    """
    Function to create graph object and store them into a python list.
    """

    # Going to use x_files as an array that always exists
    self.x_files = sorted(self.root.glob("**/*.root"))

    # Select the first N files (useful for testing)
    if self.n_files is not None:
        if self.n_files > len(self.x_files):
            print(
                f"WARNING: n_files specified ({self.n_files}) is greater than files in path given ({len(self.x_files)})"
            )

        self.x_files = self.x_files[: self.n_files]

    if len(self.x_files) == 0:
        raise RuntimeError(f"No files found in {self.root}")

    # self.root_trees = [uproot.open(f)["Tree"] for f in self.x_files]
    # Save the features, ignoring any specified
    # NOTE: We are assuming all files have the same features
    # self.features = [f for f in self.root_trees[0].keys() if f.startswith("feat_")]
    with uproot.open(self.x_files[0])["Tree"] as t:
        self.features = [f for f in t.keys() if f.startswith("feat_")]

    if self.node_features:
        # Keep only requested features
        self.discarded = [
            f for f in self.features if not f[f.find("_") + 1:] in self.node_features
        ]
        self.features = [
            f"feat_{f}" for f in self.node_features if f"feat_{f}" in self.features
        ]
    else:
        # Remove ignored features
        self.discarded = [
            f for f in self.features if f[f.find("_") + 1:] in self.ignore
        ]
        self.features = [
            f for f in self.features if f[f.find("_") + 1:] not in self.ignore
        ]
    print(f"Input node features: {self.features}")
    print(f"Discarded node features: {self.discarded}")

    # Edge and global features for geometric model
    self.edge_features = [f"edge_{f}" for f in self.edge_features]
    self.global_features = [f"glob_{f}" for f in self.global_features] if self.global_features else []
    print(f"Input edge features: {self.edge_features}")
    print(f"Input global features: {self.global_features}")
    # TODO: Preload all data as lazyarrays
    print("Preloading ROOT data...")
    self.x, self.y = preload_root_data(
        # self.root_trees,
        self.x_files,
        self.features,
        self.discarded,
        self.global_features,
    )
    print("ROOT data preloaded")

    # Need to populate a list of available training samples
    print("Populating available samples...")
    self.avail_samples = populate_avail_samples(
        self.x,
        self.y,
        self.ups_reco,
    )
    print("Available samples populated")

    # Select a subset of available samples if requested
    if self.samples and self.samples < len(self.avail_samples):
        print(f"Selecting random subset of {self.samples} samples")
        self.avail_samples = [
            self.avail_samples[i]
            for i in np.random.choice(
                len(self.avail_samples), self.samples, replace=False
            )
        ]
    elif self.samples and (self.samples >= len(self.avail_samples)):
        print(
            f"WARNING: No. samples specified ({self.samples}) exceeds number of samples loaded ({len(self.avail_samples)})"
        )

    return len(self.avail_samples)


def _process_graph(self, idx):

    file_id, evt, p_index = self.avail_samples[idx]

    x_item = self.x[file_id]
    y_item = self.y[file_id][p_index]

    evt_p_index = x_item["b_index"][evt]
    evt_leaves = x_item["leaves"][evt]
    evt_primary = x_item["primary"][evt]

    y_leaves = y_item["LCA_leaves"][evt]
    # Use this to correctly reshape LCA (might be able to just use shape of y_leaves?)
    n_LCA = y_item["n_LCA"][evt]

    # TODO: Get feature order for this file, and reorder according to global ordering
    # TODO: One-hot encode charge and any other categorical features, jks probs not

    # Get the rows of the X inputs to fetch
    # This is a boolean numpy array
    x_rows = np.logical_or(evt_p_index == 1, evt_p_index == 2) if self.ups_reco else evt_p_index == int(p_index)

    # Find the unmatched particles
    unmatched_rows = evt_p_index == -1

    if self.subset_unmatched and np.any(unmatched_rows):
        # Create a random boolean array the same size as the number of leaves
        rand_mask = np.random.choice(a=[False, True], size=unmatched_rows.size)
        # AND the mask with the unmatched leaves
        # This selects a random subset of the unmatched leaves
        unmatched_rows = np.logical_and(unmatched_rows, rand_mask)

    # Add the unmatched rows to the current decay's rows
    x_rows = np.logical_or(x_rows, unmatched_rows)

    # Here we actually load the data

    # Initialise event's X array
    x = np.empty((x_rows.sum(), len(self.features)))
    x_dis = np.empty((x_rows.sum(), len(self.discarded)))

    # And populate it
    for idx, feat in enumerate(self.features):
        x[:, idx] = x_item["features"][feat][evt][x_rows]
    for idx, feat in enumerate(self.discarded):
        x_dis[:, idx] = x_item["discarded"][feat][evt][x_rows]

    # Same for edge and global features
    x_edges = (
        compute_edge_features(
            self.edge_features,
            self.features + self.discarded,
            np.concatenate([x, x_dis], axis=1),
        )
        if self.edge_features is not []
        else []
    )
    x_global = (
        np.array(
            [
                [
                    x_item["global"][feat + f"_{p_index}"][evt]
                    for feat in self.global_features
                ]
            ]
        )
        if self.global_features != []
        else []
    )

    x_leaves = evt_leaves[x_rows]

    # Set nans to zero, this is a surrogate value, may change in future
    np.nan_to_num(x, copy=False)
    np.nan_to_num(x_edges, copy=False)
    np.nan_to_num(x_global, copy=False)

    # Normalize any features that should be
    if self.normalize is not None:
        normalize_features(
            self.normalize,
            self.features,
            x,
            self.edge_features,
            x_edges,
            self.global_features,
            x_global,
        )

    # ## Reorder LCA

    # Get LCA indices in order that the leaves appear in reconstructed particles
    # Secondaries aren't in the LCA leaves list so they get a 0
    locs = np.array(
        [
            np.where(y_leaves == i)[0].item() if (i in y_leaves) else 0
            for i in x_leaves
        ]
    )

    # Get the LCA in the correct subset order
    # If we're not allowing secondaries this is all we need
    # If we are this will contain duplicates (since secondary locs are set to 0)
    # We can't load the firs locs directly (i.e. y_item[locs, :]) because locs is (intentionally) unsorted
    y_edge = y_item["LCA"][evt].reshape((n_LCA, n_LCA)).astype(int)
    # Get the Upsilon(4S) flag
    # y_isUps = y_item["isUps"][evt]
    # Get the true mcPDG pf FSPs
    y_mass = masses_to_classes(x_item["mc_pdg"][evt][x_rows])

    # Get the specificed row/cols, this inserts dummy rows/cols for secondaries
    y_edge = y_edge[locs, :][:, locs]
    # if self.allow_secondaries:
    # Set everything that's not primary (unmatched and secondaries) rows.cols to 0
    # Note we only consider the subset of leaves that made it into x_rows
    y_edge = np.where(evt_primary[x_rows], y_edge, 0)  # Set the rows
    y_edge = np.where(evt_primary[x_rows][:, None], y_edge, 0)  # Set the columns

    # Set diagonal to -1 (actually not necessary but ok...)
    np.fill_diagonal(y_edge, -1)

    # # Ignore true LCA and masses on background events
    # if not y_isUps:
    #     y_edge.fill(-1)
    #     y_mass.fill(-1)

    n_nodes = x.shape[0]

    edge_y = torch.tensor(
        y_edge[np.eye(n_nodes) == 0],
        dtype=torch.long
    )  # Target edge tensor: shape [E]
    edge_index = torch.tensor(
        list(itertools.permutations(range(n_nodes), 2)),
        dtype=torch.long,
    )  # Fill tensor with edge indices: shape [N*(N-1), 2] == [E, 2]

    u_y = torch.tensor(
        [[1]], dtype=torch.float
    )  # Target global tensor: shape [B, F_g]

    x_y = torch.tensor(y_mass, dtype=torch.long)  # Target node tensor: shape [N]

    g = Data(
        x=torch.tensor(x, dtype=torch.float),
        edge_index=edge_index.t().contiguous(),
        edge_attr=torch.tensor(x_edges, dtype=torch.float),
        u=torch.tensor(x_global, dtype=torch.float),
        x_y=x_y,
        edge_y=edge_y,
        u_y=u_y,
    )

    return g


class BelleRecoSetGeometricInMemory(InMemoryDataset):
    def __init__(
        self,
        root,
        ups_reco=False,
        n_files=None,
        samples=None,
        subset_unmatched=True,
        features=[],
        ignore=[],
        edge_features=[],
        global_features=[],
        normalize=None,
        overwrite=False,
        **kwargs,
    ):
        """Dataset handler thingy for reconstructed Belle II MC to pytorch geometric InMemoryDataset

        This expects the files under root to have the structure:
            `root/**/<file_id>.py`
        where the root path is different for train, val, and test.
        The `**/` is to handle subdirectories, e.g. `sub00`

        The ROOT format expects the tree in every file to be named `Tree`, and all features to have the format `feat_<name>`.
        """

        assert isinstance(
            ignore, list
        ), f'Argument "ignore" must be a list and not {type(ignore)}'
        assert isinstance(
            features, list
        ), f'Argument "features" must be a list and not {type(features)}'

        self.root = Path(root)

        self.ups_reco = ups_reco

        self.normalize = normalize

        self.subset_unmatched = subset_unmatched

        self.n_files = n_files
        self.node_features = features
        self.edge_features = edge_features
        self.global_features = global_features
        self.ignore = ignore
        self.samples = samples

        print("Using LCAS format, max depth of 6 equals Ups(4S)")

        # Delete processed files, in case
        file_path = Path(self.root, "processed")
        files = list(file_path.glob("*.pt"))
        if overwrite:
            for f in files:
                f.unlink(missing_ok=True)

        # Needs to be called after having assigned all attributes
        super().__init__(root, None, None, None)

        self.data, self.slices = torch.load(self.processed_paths[0])

    @property
    def processed_file_names(self):
        return ["processed_data.pt"]

    def process(self):
        num_samples = _preload(self)
        data_list = [_process_graph(self, i) for i in range(num_samples)]
        data, slices = self.collate(data_list)
        torch.save((data, slices), self.processed_paths[0])

        del self.x, self.y, self.avail_samples, data_list, data, slices


class BelleRecoSetGeometric(Dataset):
    def __init__(
        self,
        root,
        ups_reco=False,
        n_files=None,
        samples=None,
        subset_unmatched=True,
        features=[],
        ignore=[],
        edge_features=[],
        global_features=[],
        normalize=None,
        overwrite=False,
        **kwargs,
    ):
        """Dataset handler thingy for reconstructed Belle II MC to pytorch geometric InMemoryDataset

        This expects the files under root to have the structure:
            `root/**/<file_id>.py`
        where the root path is different for train, val, and test.
        The `**/` is to handle subdirectories, e.g. `sub00`

        The ROOT format expects the tree to be named `Tree`, and all features to have the format `feat_<name>`.
        """

        assert isinstance(
            ignore, list
        ), f'Argument "ignore" must be a list and not {type(ignore)}'
        assert isinstance(
            features, list
        ), f'Argument "features" must be a list and not {type(features)}'

        self.root = Path(root)

        self.ups_reco = ups_reco

        self.normalize = normalize

        self.subset_unmatched = subset_unmatched

        self.overwrite = overwrite

        self.n_files = n_files
        self.node_features = features
        self.edge_features = edge_features
        self.global_features = global_features
        self.ignore = ignore
        self.samples = samples

        print("Using LCAS format, max depth of 6 equals Ups(4S)")

        # Delete processed files, in case
        self.file_path = Path(self.root, "processed")
        files = list(self.file_path.glob("*.pt"))
        if self.overwrite:
            for f in files:
                f.unlink(missing_ok=True)
            self.num_samples = None

        # Needs to be called after having assigned all attributes
        super().__init__(root, None, None, None)

    @property
    def processed_file_names(self):
        if self.overwrite:
            self.num_samples = _preload(self) if not self.num_samples else self.num_samples
            return [self.file_path / f'processed_data_{i}.pt' for i in range(self.num_samples)]
        else:
            return list(self.file_path.glob('processed_data_*.pt'))

    def process(self):
        for idx in range(self.num_samples):
            g = _process_graph(self, idx)
            torch.save(g, self.file_path / f"processed_data_{idx}.pt")
            del g

        del self.x, self.y, self.avail_samples

    def len(self):
        return len(self.processed_file_names)

    def get(self, idx):
        data = torch.load(self.file_path / f"processed_data_{idx}.pt")
        return data
