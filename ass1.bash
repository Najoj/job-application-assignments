#!/usr/bin/env bash

#
# Written by Johan Öhlin
# johanohlin@gmx.com
# Ran on Debian GNU/Linux 8.4 running Linux 3.14-2-amd64
# Bash version 4.3.25(1)-release
#

# Location of init's children. Init should have PID = 1.
# alternatively: cat this file, and save the list of pids.
CHILDREN="/proc/1/task/1/children"

# Logger. Saves the date and the string to print, then echoes it and puts in syslog.
function print_log () {
    # Fetch date and create the string to print
    DATE=$(date)
    STRING="$DATE:  $@"

    # Print our string to stdout and logger
    echo   "$STRING"
    logger "$STRING"
    # alternatively:  logger -s $STRING |& cat
}

# Children of init (or systemd, or whichever is PID 1)
function init_children() {
    # Prints children's PID's and put them on seperate lines with tr
    cat $CHILDREN | tr ' ' '\n' \
        | while read pid; do
        # Process name is found in comm
        cat "/proc/$pid/comm"
    done
    # alternatively: for pid in $(cat $CHILDREN); do
}

# Monitors child
function follow_child() {
    # Choose init child pid at random.
    CHILD=$(cat $CHILDREN | tr ' ' '\n' | shuf | head -1)
    echo "Keeping an eye on PID $CHILD"

    # Child's sched file, where vruntime is found
    SCHED="/proc/$CHILD/sched"
    # Child's name
    NAME=$(cat /proc/$CHILD/comm)
    
    # File exists, as long as the process is alive
    while [ -f $SCHED ]; do
        # Save se.vruntime
        VRUNTIME=$(grep 'se.vruntime' $SCHED | awk '{ print $3 }')
        # Wait a while before looking again
        sleep 10
    done

    # Print last se.vruntime
    echo "${NAME}'s last se.vruntime: $VRUNTIME"
}

# Main method. Everything in this main method will be logged.
function main() {
  init_children
  follow_child 
}

# Run the two functions an in a sane matter, print them to syslog and stdout.
main |  while read line; do
    print_log $line
done
