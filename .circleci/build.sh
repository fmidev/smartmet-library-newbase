#!/bin/bash

# Go to git repo root
cd `dirname $0`
cd ..

function insudo {
    user=`whoami`
    if [ "$user" = "root" ] ; then
	"$@"
	return
    fi
    if [ ! -x /usr/bin/sudo ] ; then
	echo "Sudo not installed and installation not possible as regular user"
	exit 1
    fi
    /usr/bin/sudo "$@"
}

jobs=`fgrep processor /proc/cpuinfo | wc -l`

set -ex
# Make sure we are using proxy, if that is needed
test -z "$http_proxy" || (
    grep -q "^proxy=" /etc/yum.conf || \
       echo proxy=$http_proxy | \
           insudo tee -a /etc/yum.conf
)

for step in $* ; do
    case $step in
	update)
	    insudo yum install -y deltarpm
	    # Update on filesystem package fails on CircleCI containers and on some else as well
	    # Enable workaround
	    insudo sed -i -e '$a%_netsharedpath /sys:/proc' /etc/rpm/macros.dist 
	    insudo yum update -y
	    ;;
	fmiprep)
		for pck in http://www.nic.funet.fi/pub/mirrors/fedora.redhat.com/pub/epel/epel-release-latest-7.noarch.rpm \
			yum-utils \
		    git \
		    ccache \
			https://download.fmi.fi/smartmet-open/rhel/7/x86_64/smartmet-open-release-17.9.28-1.el7.fmi.noarch.rpm \
			https://download.fmi.fi/fmiforge/rhel/7/x86_64/fmiforge-release-17.9.28-1.el7.fmi.noarch.rpm \
			https://download.postgresql.org/pub/repos/yum/9.5/redhat/rhel-7-x86_64/pgdg-redhat95-9.5-3.noarch.rpm ; do
			insudo yum install -y "$pck" || insudo yum reinstall -y "$pck"
		done
	    # This will speedup future steps and there seems to be
	    # wrong URLs in these in some cases
	    insudo rm -f /etc/yum.repos.d/CentOS-Vault.repo /etc/yum.repos.d/CentOS-Sources.repo
	    ;;
	cache)
	    insudo yum clean all
	    insudo rm -rf /var/cache/yum
	    insudo yum makecache
	    ;;
	deps)
	    insudo yum-builddep -y *.spec
	    ;;
	testprep)
	    sed -e 's/^BuildRequires:/#BuildRequires:/' -e 's/^#TestRequires:/BuildRequires:/' < *.spec > /tmp/test.spec
	    insudo yum-builddep -y /tmp/test.spec
	    ;;
	test)
	    make -j "$jobs" test
	    ;;
	rpm)
	    make -j "$jobs" rpm
	    mkdir -p $HOME/dist
	    for d in /root/rpmbuild $HOME/rpmbuild ; do
	    	test ! -d "$d" || find "$d" -name \*.rpm -exec mv -v {} $HOME/dist/ \; 
	    done
	    set +x
	    echo "Distribution files are in $HOME/dist:"
	    ls -l $HOME/dist
#	    test ! -d /dist || cp -av $HOME/dist /dist
	    ;;
	*)
	    echo "Unknown build step $step"
	    ;;
    esac
done
