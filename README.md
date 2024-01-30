# TARK LABORIKAPP / Smart Locker web interface

This is the readme file of an university project, where the task was to modify an existing laboratory locker by adding a smart locking feature. The web interface is used to check the opening logs of the locker, to add, remove, modify or extend locker opening rights, to check the status of the locker from distance, to open the door of the locker from distance.

## Configuration

To use this code, the repository needs to be cloned. After that a folder named "backend" has to be created inside the "src" folder. In the "backend" folder create a file called "firebase.ts" that will hold the configuration for the firebase database that holds your data and authentication.

The configuration file has to be in the following format:
```typescript
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
```
The Firebase configuration values can be obtained from the Firebase console.

## Screenshots

**Login page on web and mobile**

![image](https://github.com/siimtishler/kapp/assets/17281140/eb0fac60-5b34-4094-9d84-a8b6c9d53422)
![image](https://github.com/siimtishler/kapp/assets/17281140/cc2036bb-75d1-49bc-b6ea-cceda21fa0e3)

**Home page on web and mobile**

![image](https://github.com/siimtishler/kapp/assets/17281140/ef138ad0-3341-4b9f-b3df-f9ee6a3039c0)
![image](https://github.com/siimtishler/kapp/assets/17281140/36a58073-fcb3-4fc9-8c1b-6f95724ffff1)

**Logs page on web and mobile**

![image](https://github.com/siimtishler/kapp/assets/17281140/3835d704-27a6-47f5-9ca7-7388ecce12ea)
![image](https://github.com/siimtishler/kapp/assets/17281140/22559286-28e6-40cb-b3ef-d4a412c7ad01)

**Users page**

![image](https://github.com/siimtishler/kapp/assets/17281140/409e720f-f6c7-494b-807d-12ad205c975e)

**Settings page**

![image](https://github.com/siimtishler/kapp/assets/17281140/f45a1789-40df-4118-88ae-7859188efb60)


## Info

Further documentation and description of the web interface is available in the master branch of this repository
