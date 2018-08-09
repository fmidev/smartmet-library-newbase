pipeline {
  agent {
    node {
      label 'master'
    }

  }
  stages {
    stage('Build') {
      agent {
        docker {
          image 'fmidev'
          args '-v /tmp/shared-ccache:/ccache -v /tmp/shared-yum-cache:/var/cache/yum -v ${PWD}:/work -w /work'
          reuseNode true
        }

      }
      steps {
        sh '''
git clean -ffxd
rpmlint *.spec
mkdir -p tmp
echo %_rpmdir `pwd`/tmp > $HOME/.rpmmacros
echo %_srcrpmdir `pwd`/tmp >> $HOME/.rpmmacros
yum-builddep -y *.spec
make rpm
mkdir -p dist
find tmp -name \*.rpm | xargs -I RPM mv RPM dist/
rm -rf tmp
'''
      }
    }
    stage('Install') {
      agent {
        docker {
          image 'fmibase'
          args '-v /tmp/shared-ccache:/ccache -v /tmp/shared-yum-cache:/var/cache/yum -v ${PWD}:/work -w /work'
          reuseNode true
        }

      }
      steps {
        sh 'for i in dist/*.rpm ; do yum install -y "$i" ; done'
      }
    }
    stage('Final') {
      agent any
      steps {
        sh 'pwd ; ls -la ; docker info | tee docker.out'
      }
    }
  }
}