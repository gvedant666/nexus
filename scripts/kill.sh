sudo modprobe -r mac80211_hwsim
sudo ip netns delete machine_a
sudo modprobe -r mac80211_hwsim

echo "Stopping P2P sandbox processes..."

# 1. Kill wpa_supplicant running inside the network namespace
if ip netns list | grep -q "machine_a"; then
    echo "Terminating wpa_supplicant in namespace 'machine_a'..."
    sudo ip netns exec machine_a pkill -f "wpa_supplicant.*p2p_machine_a.conf"
fi

# 2. Kill the custom DBus daemon routing the sandbox traffic
echo "Terminating custom DBus daemon..."
sudo pkill -f "dbus-daemon --session --address=unix:path=/tmp/p2p_sandbox_dbus"

# 3. Clean up the Unix Domain Socket file if it still exists
if [ -S /tmp/p2p_sandbox_dbus ]; then
    echo "Cleaning up leftover socket file..."
    sudo rm -f /tmp/p2p_sandbox_dbus
fi

# 4. Unset the environment variable in the current session (optional but helpful)
unset DBUS_SYSTEM_BUS_ADDRESS

echo "Teardown complete."