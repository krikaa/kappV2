<template>
  <nav class="navbar">
    <div class="navbar-left">
      <span class="logo"><img src="../assets/kapp-logo13.png"> TARK LABORIKAPP</span>
    </div>
    <div class="navbar-center">
      <table class="nav-table">
        <thead>
          <tr>
            <td><router-link to="/">Kodu <i class="pi pi-home" style="font-size: 1rem;"></i></router-link></td>
            <td><router-link to="/users">Kasutajad <i class="pi pi-users" style="font-size: 1rem;"></i></router-link></td>
            <td><router-link to="/logs">Logi <i class="pi pi-file" style="font-size: 1rem;"></i></router-link></td>
            <td><router-link to="/settings">Kapi seaded <i class="pi pi-cog" style="font-size: 1rem;"></i></router-link></td>
            <td><button class="btnLogout" @click="logout" style="margin-right:0px" title="Logi välja"><i class="pi pi-sign-out" style="font-size: 1rem"></i></button></td>
          </tr>
        </thead>
      </table>
    </div>
    <div class="burger-menu" @click="toggleMenu">
      <i class="pi pi-bars" style="font-size: 1.5rem;"></i>
    </div>
    <div class="mobile-menu" :class="{ open: isMenuOpen }">
      <router-link to="/" @click="closeMenu">Kodu <i class="pi pi-home" style="font-size: 1rem;"></i></router-link>
      <router-link to="/users" @click="closeMenu" >Kasutajad <i class="pi pi-users" style="font-size: 1rem;"></i></router-link>
      <router-link to="/logs" @click="closeMenu">Logi <i class="pi pi-file" style="font-size: 1rem;"></i></router-link>
      <router-link to="/settings" @click="closeMenu">Kapi seaded <i class="pi pi-cog" style="font-size: 1rem;"></i></router-link>
      <button style="margin-bottom: 20px; margin-top: 10px" @click="() => { closeMenu(); logout(); }" value="logout" class="btnLogout" title="Logi välja"><i class="pi pi-sign-out" style="font-size: 1rem"></i></button>
    </div>
  </nav>
</template>

<script setup lang="ts">

  import { ref } from 'vue';
  import firebaseInstance from '@/backend/firebase';
  import { getAuth, signOut } from 'firebase/auth';

  const isMenuOpen = ref(false);
  
  const toggleMenu = () =>
  {
    isMenuOpen.value = !isMenuOpen.value;
  };

  const closeMenu = () =>
  {
    isMenuOpen.value = false;
  };

  const logout = () =>
  {
    signOut(getAuth(firebaseInstance))
      .then(() => console.log("Signed out"))
      .catch(err => console.log(err.message));
  }

</script>


<style scoped>
.navbar {
  display: flex;
  align-items: center;
  background-color: #242424;
  color: white;
  padding: 0.1rem;
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  white-space: nowrap;
  z-index: 1000; /* Ensure it's on top of other content */
}

.nav-table th,
.nav-table td {
  border: 0px solid #3a3a3a;
  padding-inline: 30px;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  color: #fff;
  font-family: 'Satoshi-Bold';
}

.navbar-left {
  display: flex;
  align-items: center;
  padding: 0.5rem;
  margin-left: 1rem;
  flex: 1; /* Grow to take available space */
}

.navbar-center {
  display: flex;
  margin-right: 10px;
  justify-content: right; 
  flex: 2; /* Grow to take available space */
}

.logo {
  display:flex;
  align-items:center;
  font-family: "Satoshi-Black";
  font-size: 1.5rem;
  margin-right: 1rem;
}

.logo img
{
  height: 1.5em;
  margin-inline: 0.4rem;
}

.burger-menu {
  cursor: pointer;
  margin-right: 1rem;
  flex: 0; /* Grow to take available space */
}

.mobile-menu {
  flex-direction: column;
  position: absolute;
  top: 100%;
  right: 0;
  background: #242424;
  width: 100%;
  max-height: 0;
  overflow: hidden;
  transition: max-height 0.3s ease;
  font-family:'Satoshi-Medium';
  box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1); /* Add box shadow when the menu is open */
}

.mobile-menu a {
  color: white;
  padding: 1rem;
  display: block;
  text-align: center;
  text-decoration: none;
}

.btnLogout:hover
{
  color: #b50707;
  background-color: #24242400;
}

@media (max-width: 768px) {
  .navbar-center
  {
    display:none;
  } 
  .burger-menu
  {
    display: block; /* Show the burger menu icon on small screens */
  }
  .mobile-menu.open
  {
    max-height: 300px; /* Adjust the max height to accommodate your menu items */
  }
}

@media (min-width: 769px) {
  .burger-menu
  {
    display: none; /* Hide the burger menu icon on larger screens */
  }
}

</style>


