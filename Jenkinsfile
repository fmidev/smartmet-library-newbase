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
          image 'centos:latest'
          args '-v ${PWD}:/work -w /work'
          reuseNode true
        }

      }
      steps {
        sh '''
yum install -y yum-utils ; 
yum install -y http://www.nic.funet.fi/pub/mirrors/fedora.redhat.com/pub/epel/epel-release-latest-7.noarch.rpm ; 
yum install -y https://download.fmi.fi/smartmet-open/rhel/7/x86_64/smartmet-open-release-17.9.28-1.el7.fmi.noarch.rpm ; 
yum install -y https://download.fmi.fi/fmiforge/rhel/7/x86_64/fmiforge-release-17.9.28-1.el7.fmi.noarch.rpm ; yum install -y https://download.postgresql.org/pub/repos/yum/9.5/redhat/rhel-7-x86_64/pgdg-redhat95-9.5-3.noarch.rpm ; 
yum-builddep -y *.spec ; make rpm
'''
      }
    }
    stage('Install') {
      agent {
        docker {
          image 'centos:latest'
          args '-v ${PWD}:/work -w /work'
          reuseNode true
        }

      }
      steps {
        sh 'cat Contents'
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