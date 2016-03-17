# uninstall apps
sudo apt-get -y update
sudo apt-get -y upgrade
sudo apt-get -y remove --purge lens*
sudo apt-get -y remove --purge thunderbird* 
sudo apt-get -y remove --purge firefox*
sudo apt-get -y remove --purge libreoffice*
sudo apt-get -y clean
sudo apt-get -y autoremove

# install apps
sudo apt-get -y install vim
sudo apt-get -y install git
sudo apt-get -y install tmux
sudo apt-get -y install ssh
sudo apt-get -y install open-vm-tools
sudo apt-get -y install ufw

# set firewall
sudo ufw default DENY
sudo ufw allow ssh
sudo ufw limit ssh
sudo ufw enable

# set ssh
cd 
mkdir .ssh
chmod 755 .ssh
echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDWnynAjf5B0rvPipxLibW//kson7vmVnPd2J7HKGrP1MF/p9puVmTmhCdBkiHN6VWlX+0nIgPtjj8AJlNEEP0oHXlbEj37+v61ogANcBjMjNCAexALvGc4W0LcRh+5Fu/NWnYSZXUlVV0cVOyRbf0yxB3yK+02qNTjY2goJ8Pf4KRt8xoQDtzBP5Xhix1mMJm9hPQZDJEIc1jW2kqBTZvSQC6fbzBlTOmlYQeuVLPatF65XHWoxFyvyqAtSMmmMTxOZFlo2rh8/ZeO5lN5DKuTvIJzZipAdWtPSHpvMYv/XyuMipadqVHqmiZaWWRogym2U/Mea2UVZxB3cW2wKHJd" >> .ssh/authorized_keys
chmod 755 .ssh/authorized_keys

sed -i -e "/^#PermitRootLogin/s/#//" /etc/ssh/sshd_config
sed -i -e "/^PermitRootLogin/s/yes/no/" /etc/ssh/sshd_config
sed -i -e "/^#PasswordAuthentication/s/#//" /etc/ssh/sshd_config
sed -i -e "/^PasswordAuthentication/s/yes/no/" /etc/ssh/sshd_config
sed -i -e "/^#UsePAM/s/#//" /etc/ssh/sshd_config
sed -i -e "/^UsePAM/s/yes/no/" /etc/ssh/sshd_config
