pipeline {
  agent none
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
        sh 'ls > Contents ; ls'
        sh 'git -xfd'
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