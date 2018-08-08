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
yum-builddep -y *.spec ; make rpm
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
        sh 'for i in * ; do yum install *.rom ; done'
      }
    }
    stage('Final') {
      agent any
      steps {
        sh 'pwd ; ls -la ; docker info | tee docker.out'
      }
    }
    stage('Install') {
      steps {
        dockerNode(image: 'centos:latest') {
          sh 'pwd'
        }

      }
    }
    stage('Test') {
      steps {
        dockerNode(image: 'centos:latest') {
          sh 'echo Kuuk'
        }

      }
    }
    stage('Save to repository') {
      steps {
        sh '''pwd
ls'''
      }
    }
    stage('Prepare') {
      agent any
      steps {
        sh 'pwd ; ls -la ; docker info | tee docker.out'
      }
    }
  }
}