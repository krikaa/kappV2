# TARK LABORIKAPP / Smart Locker web interface, computers and systems project

The web interface is used to check the opening logs of the locker, to add, remove, modify or extend locker opening rights, to check the status of the locker from distance, to open the door of the locker from distance.

## Configuration

To use this code, the repository needs to be cloned. After that a folder named "backend" has to be created inside the "src" folder. In the "backend" folder create a file called "firebase.ts" that will hold the configuration for the firebase database that holds your data and authentication.

The configuration file has to be in the following format:

import { initializeApp } from 'firebase/app';

const firebaseConfig =
{
   apiKey: "YOUR_API_KEY",
   authDomain: "YOUR_AUTH_DOMAIN",
   databaseURL: "YOUR_DATABASE_URL",
   projectId: "YOUR_PROJECT_ID",
   storageBucket: "YOUR_STORAGE_BUCKET",
   messagingSenderId: "YOUR_MESSAGING_SENDER_ID",
   appId: "YOUR_APP_ID",
   measurementId: "YOUR_MEASUREMENT_ID"
}
const firebaseInstance = initializeApp(firebaseConfig);
export default firebaseInstance;

The Firebase configuration values can be obtained from the Firebase console.

## Info

Further documentation and description of the web interface is available in the master branch of this repository
