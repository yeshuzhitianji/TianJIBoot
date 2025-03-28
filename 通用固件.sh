SCRIPTNAME="通用固件.sh"
RECONFIG=FALSE

HelpMsg()
{
  echo "Usage: $SCRIPTNAME [Options]"
  echo
  echo "The system environment variable, WORKSPACE, is always set to the current"
  echo "working directory."
  echo
  echo "Options: "
  echo "  --help, -h, -?        Print this help screen and exit."
  echo
  echo "  --reconfig            Overwrite the WORKSPACE/Conf/*.txt files with the"
  echo "                        template files from the BaseTools/Conf directory."
  echo
  echo Please note: This script must be \'sourced\' so the environment can be changed.
  echo ". $SCRIPTNAME"
  echo "source $SCRIPTNAME"
}

SetWorkspace()
{
  #
  # If WORKSPACE is already set, then we can return right now
  #
  export PYTHONHASHSEED=1
  if [ -n "$WORKSPACE" ]
  then
    return 0
  fi

  if [ ! -f ${SCRIPTNAME} ] && [ -z "$PACKAGES_PATH" ]
  then
    echo Source this script from the base of your tree.  For example:
    echo "  cd /Path/To/Edk2/Clone"
    echo "  . $SCRIPTNAME"
    return 1
  fi

  #
  # Check for BaseTools/BuildEnv before dirtying the user's environment.
  #
  if [ ! -f BaseTools/BuildEnv ] && [ -z "$EDK_TOOLS_PATH" ]
  then
    echo BaseTools not found in your tree, and EDK_TOOLS_PATH is not set.
    echo Please point EDK_TOOLS_PATH at the directory that contains
    echo the EDK2 BuildEnv script.
    return 1
  fi

  #
  # Set $WORKSPACE
  #
  export WORKSPACE=$PWD
  return 0
}

SetupEnv()
{
  if [ -n "$EDK_TOOLS_PATH" ]
  then
    . $EDK_TOOLS_PATH/BuildEnv
  elif [ -f "$WORKSPACE/BaseTools/BuildEnv" ]
  then
    . $WORKSPACE/BaseTools/BuildEnv
  elif [ -n "$PACKAGES_PATH" ]
  then
    for DIR in $(echo $PACKAGES_PATH | tr ':' ' ')
    do
      if [ -f "$DIR/BaseTools/BuildEnv" ]
      then
        export EDK_TOOLS_PATH=$DIR/BaseTools
        . $DIR/BaseTools/BuildEnv
        break
      fi
    done
  else
    echo BaseTools not found in your tree, and EDK_TOOLS_PATH is not set.
    echo Please check that WORKSPACE or PACKAGES_PATH is not set incorrectly
    echo in your shell, or point EDK_TOOLS_PATH at the directory that contains
    echo the EDK2 BuildEnv script.
    return 1
  fi
}

SetupPythonCommand()
{
  #
  # If PYTHON_COMMAND is already set, then we can return right now
  #
  if [ -n "$PYTHON_COMMAND" ]
  then
    return 0
  fi

  export PYTHON_COMMAND=python3
}

SourceEnv()
{
  SetupPythonCommand
  SetWorkspace
  SetupEnv
}

I=$#
while [ $I -gt 0 ]
do
  case "$1" in
    BaseTools)
      # Ignore argument for backwards compatibility
      shift
    ;;
    --reconfig)
      RECONFIG=TRUE
      shift
    ;;
    *)
      HelpMsg
      break
    ;;
  esac
  I=$((I - 1))
done

if [ $I -gt 0 ]
then
  return 1
fi

SourceEnv

unset SCRIPTNAME RECONFIG

return $?
