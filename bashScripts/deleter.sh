#!/bin/bash
# simple script to delete zombie processes if they exist and delete remaining folders and files
pkill -f mirror_client
rm -rf mirror\*
rm -rf common/
