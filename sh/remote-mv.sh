
sshpass_cmd="sshpass -p nikola123"

data_directory=socfb-caltech/N4_K1_rand/ # <-------
new_dir_name=socfb-caltech/N4_K1_rand_1/
results_directory=~/results/

mv $results_directory/$data_directory $results_directory/$new_dir_name
$sshpass_cmd ssh nikola@192.168.0.17 mv $results_directory/$data_directory $results_directory/$new_dir_name
$sshpass_cmd ssh nikola@192.168.0.26 mv $results_directory/$data_directory $results_directory/$new_dir_name
