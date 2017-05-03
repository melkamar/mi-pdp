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



if [[ -z "$1" ]]; then
        echo "Enter queue name as a parameter: [pdp_fast,pdp_long,pdp_serial]"
        exit 1
fi

queue_type="$1"

qrun2 12c 1 "${queue_type}" ./g1-serial.sh
qrun2 12c 1 "${queue_type}" ./g1-n2.sh
qrun2 12c 1 "${queue_type}" ./g1-n4.sh
qrun2 12c 2 "${queue_type}" ./g1-n8.sh
qrun2 12c 4 "${queue_type}" ./g1-n16.sh
qrun2 12c 6 "${queue_type}" ./g1-n24.sh
qrun2 12c 4 "${queue_type}" ./g1-n32.sh
qrun2 12c 6 "${queue_type}" ./g1-n60.sh

qrun2 12c 1 "${queue_type}" ./g2-serial.sh
qrun2 12c 1 "${queue_type}" ./g2-n2.sh
qrun2 12c 1 "${queue_type}" ./g2-n4.sh
qrun2 12c 2 "${queue_type}" ./g2-n8.sh
qrun2 12c 4 "${queue_type}" ./g2-n16.sh
qrun2 12c 6 "${queue_type}" ./g2-n24.sh
qrun2 12c 4 "${queue_type}" ./g2-n32.sh
qrun2 12c 6 "${queue_type}" ./g2-n60.sh

qrun2 12c 1 "${queue_type}" ./g3-serial.sh
qrun2 12c 1 "${queue_type}" ./g3-n2.sh
qrun2 12c 1 "${queue_type}" ./g3-n4.sh
qrun2 12c 2 "${queue_type}" ./g3-n8.sh
qrun2 12c 4 "${queue_type}" ./g3-n16.sh
qrun2 12c 6 "${queue_type}" ./g3-n24.sh
qrun2 12c 4 "${queue_type}" ./g3-n32.sh
qrun2 12c 6 "${queue_type}" ./g3-n60.sh