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
        sh 'ls ; ls > Contents ; ls'
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