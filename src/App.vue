<template>
    <NavBar v-if="!loading && showNavBar" />
    <TitleBar v-if="!showNavBar" />
    <router-view/>
</template>

<script setup lang="ts">
  import TitleBar from '@/components/TitleBar.vue';
  import NavBar from '@/components/NavBar.vue';
  import { onBeforeMount, ref } from 'vue';
  import router from './router';
  import { getAuth, setPersistence, browserSessionPersistence } from 'firebase/auth';
  import firebaseInstance from './backend/firebase';
  
  const showNavBar = ref(true);
  const loading = ref(true);
  const inactivityTimeout = 3600000; // 60 minutes in milliseconds
  let timer: any;

  const resetTimer = () => {
    clearInterval(timer);
    startTimer();
  };
  // To avoid abuse of session, logs out automatically after 1 hour of user inactivity on page
  const startTimer = () => {
    const auth = getAuth(firebaseInstance);
    if (auth.currentUser)
    {
      timer = setInterval(() =>
      {
        // Sign out the user after 60 minutes of inactivity
        auth.signOut();
      },
      inactivityTimeout);
    }
  };

  onBeforeMount(async () =>
  {
    const auth = getAuth(firebaseInstance);
    // To log out the user when switching tabs or sessions
    setPersistence(auth, browserSessionPersistence);
    // Checks if user is authenticated and can move to other pages, if not then redirects all requests to login page
    auth.onAuthStateChanged((user) =>
    {
      if (!user)
      {
        showNavBar.value = false; 
        router.replace('/login');
      } 
      else if (router.currentRoute.value.path === '/login')
      {
        showNavBar.value = false; // Hide NavBar on the login page
        router.replace('/');
      } 
      else
      {
        showNavBar.value = true; // Show NavBar for other routes
      }
      loading.value = false;
    });
    startTimer();
  });
  // Resets the inactivity timer when the user is active on the page
  window.addEventListener('mousemove', resetTimer);

  router.beforeEach((to, _, next) =>
  {
    if (!loading.value)
    {
      if (to.path === '/login')
      {
        showNavBar.value = false;
      } 
      else
      {
        showNavBar.value = true;
      }
    }
    next();
  });

</script>
