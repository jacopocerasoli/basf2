#!/usr/bin/env python3


from datetime import datetime, timedelta
import getpass
# requires pyjwt library which is not present by default yet
import jwt

filename = "./cdb_token"
username = "testuser"
validity = timedelta(hours=1)
secret = getpass.getpass("JWT Secret: ")

payload = {
    "user": "testuser",
    "admin": [f"user_{username}_.*", "staging_online", "online"],
    "create_iov": ["test_creation.*"],
    "exp": datetime.utcnow() + validity
}

with open(filename, "bw") as f:
    token = jwt.encode(payload, secret)
    f.write(token)
