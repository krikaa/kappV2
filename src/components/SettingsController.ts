import { getDatabase, ref, get, set, update } from 'firebase/database';
import firebaseInstance from '@/backend/firebase';

const db = getDatabase(firebaseInstance);

function stringGen(length:any)
{
  const letters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
  let randomString = '';

  for (let i = 0; i < length; i++) {
    const randomIndex = Math.floor(Math.random() * letters.length);
    randomString += letters.charAt(randomIndex);
  }

  return randomString;
}

class SettingsController
{
  static async getLockerStatus(property:any)
  {
    try
    {
      const statusRef = ref(db, `lockers/locker1/${property}`);
      const snapshot = await get(statusRef);
      return snapshot.val();
    } 
    catch (error:any)
    {
      throw new Error(`Error fetching locker status for ${property}:` + error.message);
    }
  }

  static async checkWifiStatus()
  {
    try
    {
      const lastConnectedTime = await SettingsController.getLockerStatus('wifi-last-connected');
      const lockerRef = ref(db, `lockers/locker1`);

      const currentTime = new Date().getTime();

      if (currentTime - lastConnectedTime > 600000)
      {
        try
        {
          await update(lockerRef, { 'wifi-connected': false});
        } 
        catch (error:any)
        {
          console.error(`Error updating WiFi status:`, error);
        }
      }
      else
      {
        try
        {
          await update(lockerRef, { 'wifi-connected': true});
        }
        catch (error:any)
        {
          console.error(`Error updating WiFi status:`, error);
        }
      }
      return new Date(lastConnectedTime).toLocaleString();
    }
    catch (error:any)
    {
      console.error(`Error checking WiFi status:`, error);
    }
  }

  static async checkNfcStatus()
  {
    try
    {
      const lastConnectedTime = await SettingsController.getLockerStatus('nfc-last-connected');
      const lockerRef = ref(db, `lockers/locker1`);

      const currentTime = new Date().getTime();

      if (currentTime - lastConnectedTime > 600000)
      {
        try
        {
          await update(lockerRef, { 'nfc-connected': false});
        } 
        catch (error:any)
        {
          console.error(`Error updating NFC status:`, error);
        }
      }
      else
      {
        try
        {
          await update(lockerRef, { 'nfc-connected': true});
        }
        catch (error:any)
        {
          console.error(`Error updating NFC status:`, error);
        }
      }
      return new Date(lastConnectedTime).toLocaleString();
    }
    catch (error:any)
    {
      console.error(`Error checking NFC status:`, error);
    }
  }
  
  static async toggleLockerStatus()
  {
    try
    {
      const currentStatus = (await SettingsController.getLockerStatus('solenoid-activated'));
      const lockerRef = ref(db, `lockers/locker1`);
      try
      {
        await update(lockerRef, { 'solenoid-activated': !currentStatus });
        //console.log(`Success: Toggled locker status to ${!currentStatus}`);
      }
      catch (error:any)
      {
        console.error(`Error:`, error);
      }
    }
    catch (error:any)
    {
      throw new Error(`Error toggling locker status: ${error.message}`);
    }
  }

  static async addLogEntry(name:any)
  {
    try 
    {
      const currentDate = Math.floor(new Date().getTime());
      const id = '-Nlx' + stringGen(16);
      const logEntry = 
      {
        Event: 'Opened',
        UUID: '-',
        name: name,
        type: 'teacher',
        timestamp: currentDate,
      };
      
      const logRef = ref(db, `log/${id}`);
      await set(logRef, logEntry);
    
      return { ...logEntry, id: name };
    } 
    catch (error:any) 
    {
     throw new Error('Error adding a log entry: ' + error.message);
    }
  }
  
}


  
export default SettingsController;
