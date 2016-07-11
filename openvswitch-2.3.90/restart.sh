rm -rf /var/run/openvswitch/log.txt
rmmod openvswitch
kill -9 `cat /usr/local/var/run/openvswitch/ovsdb-server.pid`
kill -9 `cat /usr/local/var/run/openvswitch/ovs-vswitchd.pid`
make install
make modules_install
modprobe gre
modprobe openvswitch
modprobe libcrc32c
rm -rf /usr/local/etc/openvswitch/conf.db
ovsdb-tool create /usr/local/etc/openvswitch/conf.db /usr/local/share/openvswitch/vswitch.ovsschema
ovsdb-server --remote=punix:/usr/local/var/run/openvswitch/db.sock \
--remote=db:Open_vSwitch,Open_vSwitch,manager_options \
--pidfile --detach --log-file
ovs-vsctl --no-wait init
ovs-vswitchd --pidfile --detach --log-file
ovs-vsctl show
