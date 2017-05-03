#!/usr/bin/env bash

#/usr/local/bin/qrun2 [12c|20c] #CPU QUEUE job_file
#
#
#12c:   Job run on nodes with 12 CPU without Hyper-Threading, 32GB RAM.
#20c:   Job run on nodes with 20 CPU without Hyper-Threading, 64GB RAM.
#     Default value: 12c
#
##CPU:  Reserve #CPU for jobs in queue QUEUE.
#
#12c:
#       Queue           | min #CPU | max #CPU | Max jobs run time
#       ================|==========|==========|=================
#       pdp_fast        |    1     |    6     |      6 min
#       pdp_long        |    1     |    6     |     30 min
#       pdp_serial      |    1     |    1     |     60 min
#
#20c:
#       Queue           | min #CPU | max #CPU | Max jobs run time
#       ================|==========|==========|=================
#       pdp_fast        |    1     |    6     |      3 min
#       pdp_long        |    1     |    6     |     15 min
#       pdp_serial      |    1     |    1     |     30 min
#
#Examples:
#         qrun2 20c 3 pdp_fast parallel_job.sh
#         qrun2 20c 1 pdp_serial serial_job.sh
#         qrun2 12c 4 pdp_long parallel_job.sh

qrun2 12c 1 pdp_long ./g1-serial.sh
qrun2 12c 1 pdp_long ./g1-n2.sh
qrun2 12c 1 pdp_long ./g1-n4.sh
qrun2 12c 2 pdp_long ./g1-n8.sh
qrun2 12c 4 pdp_long ./g1-n16.sh
qrun2 12c 6 pdp_long ./g1-n24.sh
qrun2 12c 4 pdp_long ./g1-n32.sh
qrun2 12c 6 pdp_long ./g1-n60.sh