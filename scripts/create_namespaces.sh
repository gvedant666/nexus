# 1. Load the kernel module to create 2 fake Wi-Fi radios (phy0 and phy1)
sudo modprobe mac80211_hwsim radios=1 && sudo rfkill unblock wifi && sudo rfkill unblock all

#kill fake wifi radios sudo modprobe -r mac80211_hwsim


# 2. Create two isolated network namespaces (machine_a and machine_b)
sudo ip netns add machine_a
# delete sudo ip netns delete machine_a
# sudo modprobe -r mac80211_hwsim  (destroys phy0 and phy1)
#sudo ip netns add machine_b

# 3. Move the fake radios into their respective isolated namespaces
sudo iw phy phy1 set netns name machine_a
#sudo iw phy phy1 set netns name machine_b

# 4. Turn on the loopback interfaces (required for D-Bus to work inside the namespace)
sudo ip netns exec machine_a ip link set lo up
#sudo ip netns exec machine_b ip link set lo up


# # 1. Tell NetworkManager to ignore the fake radios forever
  #sudo tee /etc/NetworkManager/conf.d/80-ignore-hwsim.conf << 'EOF'
  #[keyfile]
  #unmanaged-devices=interface-name:wlan*hwsim*;interface-name:hwsim*
  #EOF
  #
  ## 2. Restart NetworkManager to apply the rule instantly
  #sudo systemctl restart NetworkManager
  #
  ## 3. Safely load your module (No Airplane mode will trigger now!)
  #sudo modprobe mac80211_hwsim radios=1