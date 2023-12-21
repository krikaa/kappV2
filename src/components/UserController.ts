import { getDatabase, ref, get, set, update, remove } from 'firebase/database';
import firebaseInstance from '@/backend/firebase';

const db = getDatabase(firebaseInstance);

class UserController 
{

  static async getUsersAndSort(sortField:any, sortOrder:any)
  {
    try 
    {
      const userRef = ref(db, 'cards');
      const snapshot = await get(userRef);
      const users = snapshot.val();
      const formattedUsers = this.formatUsersData(users);
      const sortedUsers = this.sortTable(formattedUsers, sortField, sortOrder);
      return sortedUsers;
    } 
    catch (error:any) 
    {
      throw new Error('Error fetching and sorting users: ' + error.message);
    }
  };

  static formatUsersData(users:any) 
  {
    return Object.keys(users).map(uuid => 
    {
      const user = users[uuid];
      return { ...user, id: uuid };
    });
  };

  static sortTable(data:any, sortField:any, sortOrder:any)
  {
    const dataCopy = [...data];
    dataCopy.sort((a, b) =>
    {
      let aValue, bValue;
      if (sortField === 'owner' || sortField === 'type')
      {
        aValue = (a[sortField] || '').toLowerCase(); // Convert to lowercase for case-insensitive sorting
        bValue = (b[sortField] || '').toLowerCase();
      } 
      else if (sortField === 'date_added' || sortField === 'date_valid')
      {
        aValue = a.date_added || a.date_valid || '';
        bValue = b.date_added || a.date_valid || '';
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
  };

  static async addNewUser(owner:any, UUID:any, type:any, date_valid:any)
  {
    try 
    {
      const currentDate = Math.floor(new Date().getTime());
      const newUser = 
      {
        date_added: currentDate,
        owner: owner,
        type: type,
        date_valid: date_valid,
      };
      
      const userRef = ref(db, `cards/${UUID}`);
      await set(userRef, newUser);
    
      return { ...newUser, id: UUID };
    } 
    catch (error:any) 
    {
     throw new Error('Error adding a new user: ' + error.message);
    }
  };

  static async updateUser(userId: any, updatedUserData: any)
  {
    try
    {
      const userRef = ref(db, `cards/${userId}`);
      await update(userRef, updatedUserData);
      return updatedUserData;
    }
    catch (error:any)
    {
      throw new Error(`Error updating user: ${error.message}`);
    }
  };

  static async removeUser(UUID:any)
  {
    try
    {
      if (!UUID || UUID === '0')
      {
        throw new Error('Invalid UUID');
      }
      const userRef = ref(db, `cards/${UUID}`);
      const snapshot = await get(userRef);
      if (snapshot.exists())
      {
        await remove(userRef);
        return UUID;
      }
      else
      {
        throw new Error('User with UUID ${UUID} not found.');
      }
    }
    catch (error:any)
    {
      throw new Error('Error removing user: ' + error.message);
    }
  };
};

export default UserController;