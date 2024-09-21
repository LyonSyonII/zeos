#!/usr/bin/env bash
echo "Creating SO2 container..."
docker run -d -it -v $(pwd):/root/workspace -w /root/workspace --rm -p 1111:22 --name so2 neoxelox/so2:latest
echo "Container created! Waiting for IP assignment..."
sleep 1
ssh -X root@localhost -p 1111
echo "Sesion stopped. Stopping container..."
docker stop so2