#!/bin/bash

# surt en cas d'error
set -e

sudo apt update
sudo apt install build-essential
sudo apt install libcjson1
sudo apt install libcjson-dev
sudo apt install python3
sudo apt install sqlite3
sudo apt install libsqlite3-dev
sudo apt install python3-venv
