#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import numpy as np
try:
    import tensorflow as tf
except BaseException:
    pass
import basf2_mva
import basf2_mva_util
import time
import tempfile
import os

import tfdeploy

"""
This example shows how to save a tensorflow net with tfdeploy, so you won't need tensorflow for mva expert anymore.
"""


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):

    tf.reset_default_graph()
    # The name is needed for finding the tensor later
    x = tf.placeholder(tf.float32, [None, number_of_features], name='input')
    y = tf.placeholder(tf.float32, [None, 1])

    def layer(x, shape, name, unit=tf.sigmoid):
        with tf.name_scope(name):
            weights = tf.Variable(tf.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))), name='weights')
            biases = tf.Variable(tf.constant(0.0, shape=[shape[1]]), name='biases')
            layer = unit(tf.matmul(x, weights) + biases)
        return layer

    inference_hidden1 = layer(x, [number_of_features, number_of_features + 1], 'inference_hidden1')
    inference_activation = layer(inference_hidden1, [number_of_features + 1, 1], 'inference_sigmoid', unit=tf.sigmoid)

    epsilon = 1e-5
    inference_loss = -tf.reduce_sum(y * tf.log(inference_activation + epsilon) +
                                    (1.0 - y) * tf.log(1 - inference_activation + epsilon))

    inference_optimizer = tf.train.AdamOptimizer(learning_rate=0.01)
    inference_minimize = inference_optimizer.minimize(inference_loss)

    init = tf.global_variables_initializer()

    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.Session(config=config)

    session.run(init)
    # for making new tensorflow_op which does the same as inference_activation but using an other name.
    state = State(x, y, tf.identity(inference_activation, name='output'), inference_loss, inference_minimize, session)

    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Pass received data to tensorflow session
    """
    feed_dict = {state.x: X, state.y: y}
    state.session.run(state.optimizer, feed_dict=feed_dict)

    if epoch % 100 == 0:
        avg_cost = state.session.run(state.cost, feed_dict=feed_dict)
        print("Epoch:", '%04d' % (epoch), "cost=", "{:.9f}".format(avg_cost))
    return True


def end_fit(state):
    """
    Save tfdeploy.Model
    """
    model = tfdeploy.Model()
    model.add(state.activation, state.session)
    with tempfile.TemporaryDirectory() as path:
        filename = os.path.join(path, 'model.pkl')
        model.save(filename)
        with open(filename, 'rb') as file:
            data = file.read()

    del state
    return data


def load(obj):
    """
    Load tfdeploy.Model
    """
    with tempfile.TemporaryDirectory() as path:
        filename = os.path.join(path, 'model.pkl')
        with open(filename, 'w+b') as file:
            file.write(bytes(obj))
        model = tfdeploy.Model(filename)
    state = State()
    state.inp, state.outp = model.get('input', 'output')
    return state


def apply(state, X):
    """
    Apply estimator to passed data.
    """
    r = state.outp.eval({state.inp: X}).flatten()
    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


class State(object):
    """
    Tensorflow state
    """

    def __init__(self, x=None, y=None, activation=None, cost=None, optimizer=None, session=None):
        """ Constructor of the state object """
        #: feature matrix placeholder
        self.x = x
        #: target placeholder
        self.y = y
        #: activation function
        self.activation = activation
        #: cost function
        self.cost = cost
        #: optimizer used to minimize cost function
        self.optimizer = optimizer
        #: tensorflow session
        self.session = session


if __name__ == "__main__":
    from basf2 import conditions
    import ROOT  # noqa
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    general_options = ROOT.Belle2.MVA.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_identifier = "Simple"
    general_options.m_treename = "tree"
    variables = ['M', 'p', 'pt', 'pz',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)',
                 'chiProb', 'dr', 'dz',
                 'daughter(0, dr)', 'daughter(1, dr)',
                 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
                 'daughter(0, kaonID)', 'daughter(0, pionID)',
                 'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    specific_options = ROOT.Belle2.MVA.PythonOptions()
    specific_options.m_framework = "tensorflow"
    specific_options.m_steering_file = 'mva/examples/tensorflow/using_tfdeploy.py'
    specific_options.m_nIterations = 10
    specific_options.m_mini_batch_size = 100
    specific_options.m_normalize = True
    training_start = time.time()
    ROOT.Belle2.MVA.teacher(general_options, specific_options)
    training_stop = time.time()
    training_time = training_stop - training_start
    method = basf2_mva_util.Method(general_options.m_identifier)
    inference_start = time.time()
    test_data = ["test.root"]
    p, t = method.apply_expert(basf2_mva.vector(*test_data), general_options.m_treename)
    inference_stop = time.time()
    inference_time = inference_stop - inference_start
    auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)
    print("Tensorflow", training_time, inference_time, auc)
