#!/bin/bash

### Used for copying results back to main machine

main_machine=nikola@192.168.0.21

scp -r -q ~/results/ $main_machine:/home/nikola/