# /etc/init.d/TimeLapseInitScript.sh
### BEGIN INIT INFO
# Provides:          TimeLapseInitScript.sh
# Required-Start:    $all
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Start the timelapse program after system boot is nearly complete
# Description:       Enable service provided by daemon.
### END INIT INFO

cd /home/pi/workspace/uEye-timelapse/
sudo -u pi "./TimeLapseRun" &