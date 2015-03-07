#!/bin/bash

# Extract the full user name from the passwd file
FULL_USER_NAME=`getent passwd ${USER} | cut -d ':' -f 5 | cut -d ',' -f 1`

#set the variables name and email to FULL_USER_NAME and USER@HOST if they are not set
: ${NAME:=${FULL_USER_NAME}}
: ${EMAIL:=$USER"@"$HOST}

echo "NAME is "${NAME}
echo "EMAIL is "${EMAIL}

#replace the NAME and EMAIL field in the debian control file
cat debian.in/control.in | sed "{s?__USER_NAME__?${NAME}?}" |  sed "{s?__EMAIL__?${EMAIL}?}" > debian_from_template/control
