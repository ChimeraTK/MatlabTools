#!/bin/bash

#set the variables name and email to USER and USER@HOST if they are not set
: ${NAME:=${USER}}
: ${EMAIL:=$USER"@"$HOST}

echo "NAME is "${NAME}
echo "EMAIL is "${EMAIL}

#replace the NAME and EMAIL field in the debian control file
cat debian.in/control.in | sed "{s?__USER_NAME__?${NAME}?}" |  sed "{s?__EMAIL__?${EMAIL}?}" > debian_from_template/control
