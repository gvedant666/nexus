# dbus daemon to start session at location UNIX Domain Socket (local file based socket) and fork it
sudo dbus-daemon --session --address=unix:path=/tmp/p2p_sandbox_dbus --fork

# export for all child process to talk to private sandbox deamon instead of global linux syst Dbus and dump
# all IPC traffic directly into tmp/p2p_sandbox_dbus
export DBUS_SYSTEM_BUS_ADDRESS=unix:path=/tmp/p2p_sandbox_dbus


# -E (dont revert back to host system, be in sandbox)
# force exec inside machine_a for wpa_supplicant
# -u register services on private dbus
# Dnl80211 is modern linux netlink interface driver, talks to mac80211_hwsim virtual hardware radios
# -i wlan0 : deamon control this wifi interface inside machine_a
sudo -E ip netns exec machine_a wpa_supplicant -u -Dnl80211 -i wlan0 -c ./p2p_machine_a.conf -B