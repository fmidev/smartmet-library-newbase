pipeline {
  agent any
  stages {
    stage('Build') {
      agent {
        docker {
          image 'centos:latest'
        }

      }
      steps {
        sh 'ls > Contents ; ls'
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