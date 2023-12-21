<template>
  <div>
    <nav class="navbar">
      <span class="logo"><img class="logo-img" alt="logo" src="../assets/kapp-logo13-dark.png"> TARK LABORIKAPP</span>
    </nav>
  </div>
</template>

<style scoped>
  .navbar
  {
    display: flex;
    text-align: center;
    justify-content: center;
    background-color: #24242400;
    color: #242424;
    padding: 0.5rem;
    position: absolute;
    top: 0;
    left: 50%;
    transform: translateX(-50%);
    white-space: nowrap;
    z-index: 1000; /* Ensure it's on top of other content */
  }

  .logo
  {
    font-family: "Satoshi-Black";
    font-size: 1.5rem;
    display:flex;
    align-items:center;
  }

  .logo-img
  {
    height: 1.5em;
    margin-inline: 0.4rem;
  }

  @media (max-width: 768px)
  {
    .navbar
    {
      width: 100vw;
      color: #fff;
    }

    .logo-img
    {
      content: url('../assets/kapp-logo13.png');
    }
  }

</style>


