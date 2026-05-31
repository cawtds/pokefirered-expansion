#!/bin/bash

source /etc/profile.d/devkit-env.sh

echo "Building pokemon $1"
cd /home/pokefirered/ && make $1