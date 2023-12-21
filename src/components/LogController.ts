// LogController is a service that communicates with Firebase to retrieve the logs.
// LogController is used by ViewLog component.

import { getDatabase, ref, get, query, equalTo, orderByKey } from 'firebase/database';
import firebaseInstance from '@/backend/firebase';


const db = getDatabase(firebaseInstance);

class LogController
{
  static async getLogsAndSort(sortField: any, sortOrder: any)
  {
    try
    {
      const logRef = ref(db, 'log');
      const snapshot = await get(logRef);
      const logs = snapshot.val();

      const formattedLogs = await this.formatLogsData(logs);
      const sortedLogs = this.sortTable(formattedLogs, sortField, sortOrder);

      return sortedLogs;
    }
    catch (error: any)
    {
      throw new Error('Error fetching and sorting logs: ' + error.message);
    }
  }

  static async checkUuidType(uuid: any)
  {
    if (!uuid)
    {
      return false;
    }
    try
    {
      const userRef = ref(db, 'cards');
      const userQuery = query(userRef, orderByKey(), equalTo(uuid));
      const snapshot = await get(userQuery);
      const users = snapshot.val();
      // console.log('UUID:', uuid);
      // console.log('UUID Exists:', !!users);
  
      return !!users; // Convert truthy/falsy to boolean
    }
    catch (error: any)
    {
      throw new Error('Error checking if UUID exists with a different type: ' + error.message);
    }
  }

  static async formatLogsData(logs: any)
  {
    try
    {
      const formattedLogs = await Promise.all(
        Object.entries(logs).map(async ([_id, log]: [string, any]) =>
        {
          const uuidExists = await this.checkUuidType(log.UUID);
          return { ...log, uuidExists, };
        })
      );
  
      return formattedLogs;
    }
    catch (error: any)
    {
      throw new Error('Error fetching and formatting logs: ' + error.message);
    }
  }
  
  static sortTable(data:any, sortField:any, sortOrder:any)
  {
    const dataCopy = [...data];  
    dataCopy.sort((a, b) => 
    {
      let aValue, bValue;
      if (sortField === 'name' || sortField === 'type' || sortField === 'Event')
      {
        aValue = (a[sortField] || '').toLowerCase(); // Convert to lowercase for case-insensitive sorting
        bValue = (b[sortField] || '').toLowerCase();
      }
      else if (sortField === 'timestamp')
      {
        aValue = a.timestamp;
        bValue = b.timestamp;
      } 
      else
      {
        aValue = '';
        bValue = '';
      }
      const modifier = sortOrder === 1 ? 1 : -1;
      if (aValue < bValue) return -1 * modifier;
      if (aValue > bValue) return 1 * modifier;
      return 0;
    });
    return dataCopy;
  }
}

export default LogController;