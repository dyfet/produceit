#!/bin/sh
sudo chown root:produceit ./shellit
sudo chmod 0770 ./shellit
sudo chmod +s ./shellit
exec ./shellit $@

