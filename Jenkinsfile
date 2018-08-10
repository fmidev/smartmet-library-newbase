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
ccache -s
git clean -ffxd
rpmlint *.spec
mkdir -p tmp
echo %_rpmdir `pwd`/tmp > $HOME/.rpmmacros
echo %_srcrpmdir `pwd`/tmp >> $HOME/.rpmmacros
yum-builddep -y *.spec
make rpm
mkdir -p dist/src
mkdir -p dist/bin
find tmp -name *.src.rpm | xargs -I RPM mv RPM dist/src/
find tmp -name *.rpm | xargs -I RPM mv RPM dist/bin/
rm -rf tmp
ccache -s
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
        sh 'ls --recursive -la dist/ ; for i in dist/bin/*.rpm ; do yum install -y "$i" ; done'
      }
    }
    stage('Final') {
      steps {
        sh 'pwd ; ls --recursive -l dist/'
        archiveArtifacts(artifacts: 'dist/*', fingerprint: true, onlyIfSuccessful: true)
      }
    }
  }
}