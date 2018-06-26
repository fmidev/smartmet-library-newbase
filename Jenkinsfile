pipeline {
  agent none
  stages {
    stage('Build') {
      agent {
        docker {
          image 'centos:latest'
	  args '-v ${PWD}:${PWD} -w ${PWD}'	  
	  reuseNode true
        }
      }
      steps {
        sh 'ls > Contents ; ls'
      }
    }
    stage('Install') {
      agent {
        docker {
          image 'centos:latest'
	  args '-v ${PWD}:${PWD} -w ${PWD}'	  
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