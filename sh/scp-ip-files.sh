
dir=~/
sshpass_cmd="sshpass -p nikola123"

~/sh/cp-data.sh
$sshpass_cmd scp -q $dir/ip_addrs.txt nikola@192.168.0.17:$dir/
$sshpass_cmd scp -q $dir/ip_addrs.txt nikola@192.168.0.26:$dir/
$sshpass_cmd scp -P 8022 -q $dir/ip_addrs.txt u0_a310@192.168.0.10:/data/data/com.termux/files/home