# -*- mode: ruby -*-
Vagrant.configure("2") do |config|
config.vm.box = "debian/jessie64"
config.vm.network "private_network", type: "dhcp" 
config.vm.provision "shell", inline: <<-SHELL
apt-get update && apt-get install -y asterisk
cat <<EOF > /etc/asterisk/extensions.conf
[default]
exten => **1,1,Answer
exten => **1,2,Echo
exten => **1,3,Hangup
exten => **2,1,Answer
exten => **2,2,Milliwatt
exten => **2,3,Hangup
exten => _1XX,1,Answer
exten => _1XX,2,Dial(SIP/\${EXTEN})
exten => _1XX,3,Hangup
EOF
cat <<EOF > /etc/asterisk/sip.conf
[acc](!)
context=default
type=friend
host=dynamic
nat=yes
allow=all
secret=pass
[100](acc)
username=100
[101](acc)
username=101
EOF
/etc/init.d/asterisk restart
ip addr show eth1
SHELL
end
