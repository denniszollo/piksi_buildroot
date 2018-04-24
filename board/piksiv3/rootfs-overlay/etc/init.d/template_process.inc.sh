# /etc/init.d template for processes

# name=""
# cmd=""
# dir="/"
# user=""
# source template.inc.sh

ulimit -c unlimited

pid_file="/var/run/$name.pid"
stdout_log="/var/log/$name.log"
stderr_log="/var/log/$name.err"

get_pid()
{
    cat "$pid_file"
}

is_running()
{
    [ -f "$pid_file" ] && [ -d "/proc/`get_pid`" ] > /dev/null 2>&1
}

configure_dir_resource()
{
  local user=$1; shift
  local path=$1; shift
  local perm=$1; shift

  mkdir -p $path
  chown $user:$user $path
  chmod $perm $path
}

configure_file_resource()
{
  local user=$1; shift
  local path=$1; shift
  local perm=$1; shift

  touch $path
  chown $user:$user $path
  chmod $perm $path
}

_setup_permissions()
{
    if type setup_permissions | grep -q "shell function"; then
        setup_permissions
    fi
}

case "$1" in
    start)
    if is_running; then
        echo "Already started"
    else
        echo "Starting $name"
        _setup_permissions
        cd "$dir"
        if [ -z "$user" ]; then
            sudo $cmd >> "$stdout_log" 2>> "$stderr_log" &
        else
            sudo -u "$user" $cmd >> "$stdout_log" 2>> "$stderr_log" &
        fi
        echo $! > "$pid_file"
        if ! is_running; then
            echo "Unable to start, see $stdout_log and $stderr_log"
            exit 1
        fi
    fi
    ;;
    stop)
    if is_running; then
        echo -n "Stopping $name.."
        kill `get_pid`
        for i in {1..10}
        do
            if ! is_running; then
                break
            fi

            echo -n "."
            sleep 1
        done
        echo

        if is_running; then
            echo "Not stopped; may still be shutting down or shutdown may have failed"
            exit 1
        else
            echo "Stopped"
            if [ -f "$pid_file" ]; then
                rm "$pid_file"
            fi
        fi
    else
        echo "Not running"
    fi
    ;;
    restart)
    $0 stop
    if is_running; then
        echo "Unable to stop, will not attempt to start"
        exit 1
    fi
    $0 start
    ;;
    status)
    if is_running; then
        echo "Running"
    else
        echo "Stopped"
        exit 1
    fi
    ;;
    *)
    echo "Usage: $0 {start|stop|restart|status}"
    exit 1
    ;;
esac

exit 0

