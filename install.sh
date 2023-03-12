#!/bin/bash
sudo cp *.h /usr/local/include/.
sudo cp dist/Debug/GNU-Linux/*.a /usr/local/lib/.
#sudo cp dist/Debug/GNU-Linux/*.so /usr/local/lib/.
sudo ldconfig -v
