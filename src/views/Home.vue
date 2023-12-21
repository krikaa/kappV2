<template>
  <div id="home-page">
    <img src="@/assets/nutikapp.jpg" alt="TARK LABORIKAPP" class="rounded-image mx-auto" />
    <div class="row">
      <button v-ripple="{ pt: { root: { style: 'background: rgba(57, 172, 231, 0.9)' }}}" @click="navigateTo('/logs')" class="p-ripple button">Vaata logi</button>
    </div>
    <div class="row">
      <button @click="navigateTo('/users')" v-ripple="{ pt: { root: { style: 'background: rgba(57, 172, 231, 0.9)' }}}" class="p-ripple button">Autoriseeritud kasutajad</button>
    </div>
    <div class="row">
      <button @click="navigateTo('/settings')" v-ripple="{ pt: { root: { style: 'background: rgba(57, 172, 231, 0.9)' }}}" class="p-ripple button">Kapi seaded</button>
    </div>
  </div>
</template>

<script setup lang="ts">

  import router from '@/router';
  import { onMounted } from 'vue';

  onMounted (() =>
  { 
    // Changes document body class to disable overflow scroll bug
    document.body.classList.remove('login-overflow');
    document.body.classList.add('regular-overflow');
  });

  const navigateTo = (path: string) =>
  {
    router.push({ path });
  };
  
</script>

<style scoped>

  .rounded-image
  {
    width: 100%; 
    max-width: 250px;
    height: auto;
    border-radius: 16px;
    margin-bottom: 15px;
    margin-top:10px;
  }

</style>
