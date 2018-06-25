pipeline {
  agent {
    docker {
      image 'centos:latest'
    }

  }
  stages {
    stage('Build') {
      steps {
        dockerNode(image: 'centos:latest') {
          sh 'ls'
        }

      }
    }
  }
}