<template>
  <div id="settings-page">
    <div class="log-box" style="display:flex; flex-direction: column;">
      <div>
        <h1 id="settings-title">Laborikapp</h1>
      </div>
      <div>
        <div class="setting-row">
          <h1 class="setting-table-title" title="WiFi ühenduse staatus">WiFi staatus: </h1>  
          <h1 :title="wifiTitle" class="setting-table-status">{{ wifiText }}</h1>
        </div>
        <div class="setting-row">
          <h1 class="setting-table-title" title="NFC lugeri staatus">NFC staatus: </h1>
          <h1 :title="nfcTitle" class="setting-table-status">{{ nfcText }}</h1>
        </div>
        <div class="setting-row">  
          <h1 class="setting-table-title" title="Solenoidi olek">Luku olek: </h1>
          <i class="pi lock-icon" :class="lockIcon" :title="statusTitle" 
            :style="[isLocked ? { color: 'red' } : { color: 'lightgreen' }]">
          </i>
        </div> 
        <div class="setting-row"> 
          <h1 class="setting-table-title" title="Magnetanduri määratud ukse olek">Ukse olek: </h1>
          <h1 class="setting-table-status">{{ solenoidText }}</h1>
        </div>  
      </div>
      <br />
      <div style="display:flex; width:100%">
        <div style="width:25%"></div>
        <div style="display:flex; justify-content:center; width: 50%">
          <button v-ripple="{ pt: { root: { style: 'background: rgba(57, 172, 231, 0.9)' }}}" :title="btnTitle" class="p-ripple button" @click="handleToggleLock" style="display: flex; align-items: center">
            <i class="pi pi-unlock"></i>
            <p id="unlock-btn" style="width:80%">{{ buttonText }} </p>
            <div style="width:10%"></div>
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">

  import { ref, onMounted, onBeforeUnmount, computed } from 'vue';
  import SettingsController from '@/components/SettingsController';
  import firebaseInstance from '@/backend/firebase';
  import { getAuth } from 'firebase/auth';
  
  const isLocked = ref(true);
  const isOpen = ref(true);
  const isOnline = ref(true);
  const nfcConnected = ref(true);
  const statusTitle = ref('Lukus');
  const btnTitle = ref('');
  const wifiTitle = ref('');
  const nfcTitle = ref('');
  const lockIcon = ref('pi-lock');
  const buttonClass = ref('button');
  const createText = (condition:any, trueText:string, falseText:string) => computed(() => condition.value ? trueText : falseText);
  const solenoidText = createText(isOpen, 'Avatud', 'Suletud');
  const buttonText = createText(isLocked, 'Ava kapp', 'Lukusta kapp');
  const wifiText = createText(isOnline, 'Ühendatud', 'Pole ühendatud');
  const nfcText = createText(nfcConnected, 'Ühendatud', 'Pole ühendatud');

  // Checks status of locker and modifies the reactive variables to reflect the new values in realtime
  const checkLockerStatus = async () =>
  {
    const checks = [
      { prop: 'solenoid-activated', ref: isLocked, expected: false },
      { prop: 'door-open', ref: isOpen, expected: true},
      { prop: 'wifi-connected', ref: isOnline, expected: true },
      { prop: 'nfc-connected', ref: nfcConnected, expected: true },
    ];
    try
    {
      for (const { prop, ref, expected } of checks)
      {
        try
        {
          ref.value = (await SettingsController.getLockerStatus(prop)) === expected;
        }
        catch (error:any)
        {
          console.error(`Error fetching ${prop} status:`, error);
        }
      }
      wifiTitle.value = 'Viimati ühenduses: ' + await SettingsController.checkWifiStatus();
      nfcTitle.value = 'Viimati ühendatud: ' + await SettingsController.checkNfcStatus();
    }
    catch (error:any)
    {
      console.error(`Error fetching WiFi/NFC`, error);
    }
    updateComputedProperties();
  };

  const updateComputedProperties = () =>
  {
    statusTitle.value = isLocked.value ? 'Lukus' : 'Avatud';
    btnTitle.value = isLocked.value ? 'Ava kapp' : 'Lukusta kapp';
    lockIcon.value = isLocked.value ? 'pi-lock' : 'pi-lock-open';
    buttonClass.value = isLocked.value ? 'button' : 'button-locked';
  };
  // Handles communication with the SettingsController service to activate or deactivate the solenoid-lock
  const handleToggleLock = async () =>
  {
    try
    {
      await SettingsController.toggleLockerStatus();
      checkLockerStatus(); 
    } 
    catch (error:any)
    {
      console.error(error);
    }
    if (isLocked.value)
    {
      try
      {
        const user = getAuth(firebaseInstance).currentUser;
        console.log(user?.email);
        const displayName = user && user.email ? user.email.replace('@nutikapp.ee', ' WEB') : 'Admin';
        await SettingsController.addLogEntry(displayName);
      }
      catch (error:any)
      {
        console.error('Error adding log', error);
      }
    }
  };

  onMounted(() =>
  {
    document.body.classList.remove('login-overflow');
    document.body.classList.add('regular-overflow');
    checkLockerStatus();
    // Checks locker status every 10 seconds
    const intervalId = setInterval(checkLockerStatus, 10000);

    onBeforeUnmount(() =>
    {
      // Clear the interval to avoid checking locker status when on other views
      clearInterval(intervalId);
    });
  });

</script>

<style scoped>

  .time-input
  {
    width: 100%;
    margin-left: 5px;
    text-align: center;
    position: relative; /* Add relative positioning to the container */

    /* Set the background color and border for the input */
    background-color: #101010;
    border: 1px solid rgb(100, 100, 100);
    border-radius: 4px;
    outline:none;
  }

  .setting-row
  {
    display: flex;
    justify-content: space-between;
    margin-bottom: 10px; /* Adjust as needed */
  }

  .pi-unlock
  {
    font-size: 2rem;
    margin-right: 10px;
    text-align:left; 
    width:10%;
  }

  .lock-icon
  {
    font-size: 1.8rem;
    /* padding-block: 8px; */
    align-items: center;
    display:flex;
    padding-inline: 20px;
    width:250px;
  }

  #settings-title
  {
    font-family:'Satoshi-Black';
    font-size: 1.8rem;
    padding: 0px;
    margin-top: -15px;
    margin-bottom:25px;
    white-space: nowrap;
  }

  .log-box
  {
    background-color: #101010;
    padding: 30px;
    border-radius: 8px;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    max-width: 95%;
  }

  .setting-table-title
  { 
    text-align: right;
    padding-inline: 20px;
    width: 250px;
    text-wrap: nowrap;
    font-family: 'Satoshi-Medium';
    font-size: clamp(1rem, 1.5rem, 2rem);
    white-space: nowrap;
  }

  .setting-table-status
  { 
    text-align: left;
    width: 250px;
    text-wrap: nowrap;
    padding-inline: 20px;
    font-family: 'Satoshi-Medium';
    font-size: clamp(1rem, 1.5rem, 2rem);
    white-space: nowrap;
  }

  /* Base styles for the range slider */
input[type="range"] {
  -webkit-appearance: none;
  appearance: none;
  width: 99%;
  height: 10px;
  background-color: #101010;
  border: 1px solid rgb(100,100,100);
  outline: none;
  cursor: pointer;
  border-radius: 15px;
}

/* Styling the slider thumb (circle) */
input[type="range"]::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 20px;
  height: 20px;
  background-color: #fff;
  border-radius: 99%;
  cursor: pointer;
  position: relative;
  z-index: 1;
}

/* Hover styles for the range slider */
input[type="range"]:hover {
  background-color: #242424;
  border-color: #242424;
}

/* Hover styles for the slider thumb (circle) */
input[type="range"]:hover::-webkit-slider-thumb {
  background-color: #39ace7;
}

@media (max-width: 768px)
{
  .log-box
  {
    width: 95vw;
    padding-inline: 10px;
  }
  .pi
  {
    font-size: 1.2rem;
  }
  .pi-unlock
  {
    font-size: 1.4rem;
  }

  #settings-title
  {
    font-size:1.6rem;
  }

  .setting-table-status
  {
    font-size: 1.2rem;
    width: 160px;
  }

  .setting-table-title
  {
    font-size: 1.2rem;
    width: 160px;
  }

  .lock-icon
  {
    font-size:1.5rem;
    width: 160px;
  }

  #unlock-btn
  {
    font-size: 1rem;
  }

}

</style>