#!/usr/bin/env bash

cat << EOF
{
  "timestamp": "$VOY_EVENT_TIME",
  "route": "$VOY_ROUTE_NAME",
  "batch_size": "$VOY_BATCH_SIZE",
  "event_type": "$VOY_EVENT_TYPE",
  "path": "$VOY_EVENT_PATH",
  "pid": "$$"
}
EOF
