<template>
  <div id="user-page">
    <div style="flex-grow:1; width:100%; max-height: 95%;">
      <div class="flex-parent-element">
        <div class="flex-child-element-button">
          <button v-ripple="{ pt: { root: { style: 'background: rgba(57, 172, 231, 0.9)' }}}" class="p-ripple new-button" @click="openAddDialog" title="Lisa kasutaja">
            <i class="pi pi-user-plus" style="font-size: 1.2rem"></i>
          </button>
        </div>
        <div class="flex-child-element-title">
          <label class="lbl-title">Autoriseeritud kasutajad</label>
        </div>
      </div>
    <div class="log-box">
      <div class="table-wrapper scrollbar">
        <table class="logs-table">
          <thead class="fixed-header">
            <tr>
              <th class="checkbox-column">
                <div class="checkbox-container">
                  <input title="Vali kõik" type="checkbox" v-model="selectAll" @change="selectAllRows" />
                  <button title="Eemalda valitud" class="delete-button" @click="openRemoveDialog('deleteSelected')" v-show="selectedRows.length > 0">
                  <i class="pi pi-trash" style="font-size: 1rem;"></i>  
                  </button>
                </div>
              </th>
              <th>UUID</th>
              <th title="Sorteeri: nimi" class="sortable" @click="sortTable('owner')">
                Nimi  <i v-if="isOwnerSorted === 1" class="pi pi-sort-alpha-down"></i>
                <i v-else-if="isOwnerSorted === -1" class="pi pi-sort-alpha-down-alt"></i>
              </th>
              <th title ="Sorteeri: tüüp" class="sortable" @click="sortTable('type')">
                Tüüp  <i v-if="isTypeSorted === 1" class="pi pi-sort-alpha-down"></i>
                <i v-else-if="isTypeSorted === -1" class="pi pi-sort-alpha-down-alt"></i>
              </th>
              <th title="Sorteeri: kuupäev" class="sortable" @click="sortTable('date_added')">
                Lisatud:</th>
              <th title="Sorteeri: kuupäev" class="sortable" @click="sortTable('date_valid')">Aegub:</th>
              <th >Tegevused</th>
            </tr>
          </thead>
          <tbody v-if="authUsers.length > 0">
            <tr v-for="user in authUsers" :key="user.id" :class="{ 'selected-row': selectedRows.includes    (user.id) }" class="logs-row">
              <td class="checkbox-column" >
                <input title="Vali" type="checkbox" v-model="selectedRows" :value="user.id" />
              </td>
              <td >{{ user.id }}</td>
              <td >{{ user.owner }}</td>
              <td >{{ user.type }}</td>
              <td >{{ formatTimestamp(user.date_added) }}</td>
              <td  :style="{ color: isUserExpired(user.date_valid) ? 'red' : '#fff' }">{{ formatTimestamp(user.date_valid) }}</td>
              <td>
                <button title="Muuda andmeid" @click="openModifyDialog(user)" class="user-button"><i class="pi pi-user-edit" style="font-size: 1rem;"></i></button>
                <button title="Eemalda kasutaja" @click="openRemoveDialog('handleRemoveUser', user)" class="remove-button">
                    <i class="pi pi-trash" style="font-size: 1rem;"></i>
                </button>
              </td>
            </tr>
          </tbody>
          <tbody v-else>
            <br>
            <tr>
              <td colspan="7">Autoriseeritud kasutajad puuduvad. Kasutajaid saab lisada vasakult ülemisest  nurgast.</td>
            </tr>
          </tbody>
        </table>
      </div>
    </div>
  </div>
  </div>
  <!-- Pop-up dialog for adding a user to the authorized users -->
  <dialog ref="addDialog" class="user-dialog">
      <div style="display:inline-flex; width:100%">
        <div style="width:25%"></div>
        <div style="width:50%">
          <h1 id="header-dialog">Uus kasutaja</h1>
        </div>
        <div style="width:25%; text-align: right">
          <button id="x-close">
            <i class="pi pi-times" style="font-size: 1.2rem" @click="closeAddDialog"></i>
          </button>
        </div>
      </div>
      <div style="display: flex; flex-direction: column; align-items: center">  
        <div style="text-align:left; width:90%">
          <label class="label-input">Täisnimi <span class="text-danger">*</span></label>
          <input class="dialog-input" :class="{ 'error-input': v$.ownerInput.$error}" v-model="ownerInput" type="text" id="owner" placeholder="Sisesta kasutaja nimi" autocomplete="name"/>
          <div v-if="v$.ownerInput.$dirty && v$.ownerInput.required.$invalid" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> {{v$.ownerInput.required.$message }}</div>
          <div v-if="v$.ownerInput.$dirty && v$.ownerInput.maxLength.$invalid" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> {{v$.ownerInput.maxLength.$message }}</div>
          <div v-if="v$.ownerInput.$dirty && v$.ownerInput.isName.$invalid" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> {{v$.ownerInput.isName.$message }}</div>
        </div>
        <div style="text-align:left; width:90%">
          <label class="label-input">UUID <span class="text-danger">*</span></label>
          <input v-maska:[options] data-maska="HH HH HH HH HH HH HH" class="dialog-input" :class="{ 'error-input': v$.uuidInput.$error}" v-model="uuidInput" name="uuidInput" type="text" id="UUID" placeholder="Sisesta kaardi UUID" autocomplete="none"/>
          <div v-if="v$.uuidInput.$dirty && v$.uuidInput.required.$invalid" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> {{v$.uuidInput.required.$message }}</div>
          <div v-if="v$.uuidInput.$dirty && v$.uuidInput.isHex.$invalid" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> {{ v$.uuidInput.isHex.$message }}</div>
          <div v-if="v$.uuidInput.$dirty && v$.uuidInput.isUnique.$invalid" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> {{ v$.uuidInput.isUnique.$message }}</div>
          <div v-if="v$.uuidInput.$dirty && v$.uuidInput.maxLength.$invalid" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> {{ v$.uuidInput.maxLength.$message }}</div>
          <div v-if="v$.uuidInput.$dirty && v$.uuidInput.minLength.$invalid" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> {{ v$.uuidInput.minLength.$message }}</div>
        </div>
        <div style="text-align:left; width:90%">
          <label class="label-input">Kasutaja tüüp <span class="text-danger">*</span></label>
          <select id="type" :class="{ 'error-input': v$.typeInput.$error}" v-model="typeInput">
            <option value="student">student</option>
            <option value="teacher">teacher</option>
          </select>
          <div v-if="v$.typeInput.$error" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> See väli on kohustuslik</div>
        </div>
        <div style="text-align:left; width:90%">
          <label class="label-input">Kehtivuse lõpp <span class="text-danger">*</span></label>
          <input class="dialog-input" :class="{ 'error-input': v$.dateInput.$error}" v-model="dateInput" name="uuidInput" type="date" id="UUID" placeholder="Vali kuupäev">
          <div v-if="v$.dateInput.$error" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> See väli on kohustuslik</div>
        </div>
      </div>
      <div>
        <button v-ripple="{ pt: { root: { style: 'background: rgba(57, 172, 231, 0.9)' }}}" class="p-ripple dialog-button" @click="handleAddUser">Lisa</button>
      </div>
    </dialog>
    <!-- Pop up dialog for modifying selected user's data -->
    <dialog ref="modifyDialog" class="user-dialog">
      <div style="display:inline-flex; width:100%">
        <div style="width:25%"></div>
        <div style="width:50%">
          <h1 id="header-dialog">Muuda andmeid</h1>
        </div>
        <div style="width:25%; text-align: right">
          <button id="x-close">
            <i class="pi pi-times" style="font-size: 1.2rem" @click="closeModifyDialog"></i>
          </button>
        </div>
      </div>
      <div style="display: flex; flex-direction: column; align-items: center">  
        <div style="text-align:left; width:90%">
          <label class="label-input">Täisnimi <span class="text-danger">*</span></label>
          <input class="dialog-input" :class="{ 'error-input': v$a.ownerInput.$error}" v-model="ownerInput" type="text" id="owner" placeholder="Sisesta kasutaja nimi"/>
          <div v-if="v$a.ownerInput.$dirty && v$a.ownerInput.required.$invalid" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> {{ v$a.ownerInput.required.$message }}</div>
          <div v-if="v$a.ownerInput.$dirty && v$a.ownerInput.maxLength.$invalid" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> {{ v$a.ownerInput.maxLength.$message }}</div>
          <div v-if="v$a.ownerInput.$dirty && v$a.ownerInput.isName.$invalid" class="error"><i class="pi pi-info-circle" style="font-size: 0.7rem"></i> {{ v$a.ownerInput.isName.$message }}</div>
        </div>
        <div style="text-align:left; width:90%">
          <label class="label-input">UUID <span class="text-danger">*</span></label>
          <input class="dialog-input" style="pointer-events: none" v-model="uuidInputMod" name="uuidInputMod" type="text" id="UUID" placeholder="Sisesta kaardi UUID" readonly/>
        </div>
        <div style="text-align:left; width:90%">
          <label class="label-input">Kasutaja tüüp <span class="text-danger">*</span></label>
          <select id="type" v-model="typeInput">
            <option value="student">student</option>
            <option value="teacher">teacher</option>
          </select>
        </div>
        <div style="text-align:left; width:90%">
          <label class="label-input">Kehtivuse lõpp <span class="text-danger">*</span></label>
          <input class="dialog-input" v-model="dateInput" name="dateInput" type="date" id="UUID" placeholder="Vali kuupäev">
        </div>
      </div>
      <div>
        <button v-ripple="{ pt: { root: { style: 'background: rgba(57, 172, 231, 0.9)' }}}" class="p-ripple dialog-button" @click="handleModifyUser">Salvesta</button>
      </div>
    </dialog>
    <!-- Pop up dialog for confirming removal of user(s) -->
    <dialog ref="removeDialog" class="user-dialog" style="padding-bottom: 20px; padding-inline: 20px">
      <div style="display:inline-flex; width:100%">
        <div style="width:15%"></div>
        <div style="width:70%">
          <h1 style="font-family:'Satoshi-Black'; font-size: 1.3rem; margin-bottom: 20px">Kustuta kasutaja(d)</h1>
        </div>
        <div style="width:15%; text-align: right">
          <button id="x-close">
            <i class="pi pi-times" style="font-size: 1.2rem" @click="closeRemoveDialog"></i>
          </button>
        </div>
      </div>  
      <div style="font-family:'Satoshi-Medium'">Kas oled kindel, et soovid valitud kasutaja(d) eemaldada?</div>
      <div style="display:flex">
        <button v-ripple="{ pt: { root: { style: 'background: rgba(57, 172, 231, 0.9)' }}}" class="p-ripple dialog-button" @click="handleRemoval(selectedActionType)">Jah</button>
        <button v-ripple="{ pt: { root: { style: 'background: rgba(57, 172, 231, 0.9)' }}}" class="p-ripple dialog-button" @click="closeRemoveDialog">Ei</button>
      </div>
    </dialog>
</template>

<script setup lang="ts">

  import { ref, onMounted, computed, reactive } from 'vue';
  import { useVuelidate } from '@vuelidate/core';
  import { required, minLength, maxLength, helpers } from '@vuelidate/validators';
  import UserController from '@/components/UserController';
  import router from '@/router';

  const selectedUser = ref<any>(null); 
  const selectedActionType = ref('');
  const ownerInput = ref('');
  const uuidInput = ref('');
  const uuidInputMod = ref('');
  const typeInput = ref('student');
  const dateInput = ref(new Date().toISOString().split('T')[0]);

  const addDialog = ref<HTMLDialogElement | null>(null);
  const modifyDialog = ref<HTMLDialogElement | null>(null);
  const removeDialog = ref<HTMLDialogElement | null>(null);   
  const authUsers = ref<{ id: string; owner: string; type: string; date_added: number; date_valid: number }[]>([]);
  const selectedRows = ref<string[]>([]);
  const selectAll = ref(false);
  const sortField = ref('date_added');
  const sortOrder = ref(0);
  const isOwnerSorted = computed(() => sortField.value === 'owner' ? sortOrder.value : 0);
  //const isDateSorted = computed(() => sortField.value === 'date_added' ? sortOrder.value : 0);
  const isTypeSorted = computed(() => sortField.value === 'type' ? sortOrder.value : 0);
  const isUserExpired = (timestamp:any) =>
  {
    return timestamp < Date.now();
  };

  const isUnique = (value: string) =>
  {
    return authUsers.value.every(user => user.id !== value);
  };
  const isHex = (value: string) =>
  {
    if (value.length < 1 || value.length > 20)
    {
      return true; 
    }
    const hexParts = value.split(' ');
    return hexParts.every(part => /^[0-9A-F]{2}$/.test(part));
  };
  const isName = (value: string) =>
  { 
    if (value.length < 1 || value.length > 100)
    {
      return true;
    }
    return /^[a-zA-Z]+([ -][a-zA-Z]+)*$/.test(value);
  }
  // Rules for different inputs for input validation
  const rules = 
  {
    ownerInput:
    { 
      required: helpers.withMessage('See väli on kohustuslik.', required),
      maxLength: helpers.withMessage('Sisestatud nimi on liiga pikk', maxLength(100)),
      isName: helpers.withMessage('Nimes on sobimatu sümbol. Kontrollige andmeid.', isName)
    },
    uuidInput:
    { 
      required: helpers.withMessage('See väli on kohustuslik.', required), 
      minLength: helpers.withMessage('Sisestatud UUID on liiga lühike.', minLength(2)), 
      maxLength: helpers.withMessage('Sisestatud UUID on liiga pikk.', maxLength(20)), 
      isHex:helpers.withMessage('Vigane formaat. Kontrollige andmeid.', isHex), 
      isUnique: helpers.withMessage('Sisestatud UUID on juba andmebaasis olemas.', isUnique)
    },
    typeInput: { required },
    dateInput: { required }
  };

  const v$ = useVuelidate(rules, 
  {
    ownerInput,
    uuidInput,
    typeInput,
    dateInput,
  });
  // Different vuelidate object for the second modal to avoid conflict
  const v$a = useVuelidate(
  { 
    ownerInput: 
    { 
      required: helpers.withMessage('See väli on kohustuslik', required),
      maxLength: helpers.withMessage('Sisestatud nimi on liiga pikk.', maxLength(100)),
      isName: helpers.withMessage('Nimes on sobimatu sümbol. Kontrollige andmeid.', isName)
    }
  },
  { ownerInput });
  // Options for the input mask to allow only the correct UUID format
  const options = reactive(
  {
    tokens: 
    {
      H: 
      {
        pattern: /[0-9A-Fa-f]/,
        transform: (chr:string) => chr.toUpperCase()
      }
    },
    eager: false
  });
  // To open the dialog to add users
  const openAddDialog = (log: any = null) =>
  {   
    if(addDialog.value)
    {
      v$.value.$reset();
      ownerInput.value = '';
      uuidInput.value = log ? log.UUID : '';
      typeInput.value = 'student';
      dateInput.value = getNextDate().toISOString().split('T')[0];
      addDialog.value.showModal();
    }
  };
  // For closing the dialog for adding users
  const closeAddDialog = () =>
  {
    if (addDialog.value)
    {
      addDialog.value.close();
    }
  };
  // Returns the next correct end of the semester date that will be set as default
  const getNextDate = (): Date =>
  {
    const currentDate = new Date();
    const currentMonth = currentDate.getMonth() + 1;
    const currentDay = currentDate.getDate();
    let targetDate: Date;
    if ((currentDay == 31 && currentMonth == 1) || (currentMonth >= 2 && currentMonth <= 6))
    {
      targetDate = new Date(new Date().setMonth(5, 30));
    }
    else if ((currentDay == 30 && currentMonth == 6) || (currentMonth > 6 && currentMonth <= 12))
    {
      targetDate = new Date(new Date().setFullYear(new Date().getFullYear() + 1, 0, 31));
    }
    else if (currentMonth == 1)
    {
      targetDate = new Date(new Date().setMonth(0, 31));
    }
    else
    {
      targetDate = new Date(new Date().setMonth(new Date().getMonth() + 5));
    }
    console.log(currentDay, currentMonth);
    return targetDate;
  };
  // For opening the user removal confirmation dialog
  const openRemoveDialog = (actionType:any, user?:any) =>
  {
    if (removeDialog.value)
    {
      removeDialog.value.showModal();
    }
    if (user)
    {
      selectedUser.value = user;
    }
    selectedActionType.value = actionType;
  };
  // For closing the user removal confirmation dialog
  const closeRemoveDialog = () =>
  {
    if (removeDialog.value)
    {
      removeDialog.value.close();
      selectedActionType.value = 'deleteSelected';
    }
  };
  // Checks if type of removal is Single user or multi user and handles it accordingly
  const handleRemoval = (selectedActionType:any) =>
  {
    //console.log(selectedUser.value);
    const userId = selectedUser.value ? selectedUser.value.id: null;
    if (selectedActionType === 'deleteSelected')
    {
      //console.log('deleteSelected');
      deleteSelectedRows();
    }
    else if (selectedActionType === 'handleRemoveUser')
    {
      //console.log('handleRemove');
      handleRemoveUser(userId);
    }
    // Close the dialog after handling the removal
    closeRemoveDialog();
  };
  // For opening the dialog to modify existing users
  const openModifyDialog = (user:any) =>
  {
    if (modifyDialog.value)
    {
      v$a.value.$reset();
      selectedUser.value = user;
      ownerInput.value = user.owner;
      uuidInputMod.value = user.id;
      typeInput.value = user.type;
      dateInput.value = new Date(user.date_valid).toISOString().split('T')[0];

      modifyDialog.value.showModal();
    }
  };
  // Closes the dialog to modify existing users
  const closeModifyDialog = () =>
  {
    if(modifyDialog.value)
    {
      selectedUser.value = null;
      modifyDialog.value.close();
    }
  };
  // Formats the timestamp to show only day, month and year in local format
  const formatTimestamp = (timestamp: any) => 
  {
    const date = new Date(timestamp);
    const options: Intl.DateTimeFormatOptions = 
    {
      year: 'numeric',
      month: '2-digit',
      day: '2-digit',
    };
    return date.toLocaleString('et-EE', options);
  }; 
  // Handles communication with UserController service to add users to the database
  const handleAddUser = async () =>
  {
    if (v$.value.$invalid)
    {
      v$.value.$touch();
      return;
    }
    try
    {
      const newUser = await UserController.addNewUser(ownerInput.value, uuidInput.value, typeInput.value, new Date(dateInput.value).getTime());
      authUsers.value.push(newUser);
      closeAddDialog();
    } 
    catch (error:any)
    {
      console.error('Error adding a new user:', error.message);
    }
  };
  // Handles communication with UserController to modify existing user
  const handleModifyUser = async () =>
  {
    if (v$a.value.$invalid)
    {
      console.log(v$a.value)
      v$a.value.$touch();
      return;
    }
    try
    {
      const userId = selectedUser.value.id;
      const updatedUserData =
      {
        owner: ownerInput.value,
        type: typeInput.value,
        date_valid: new Date(dateInput.value).getTime(),
      };

      await UserController.updateUser(userId, updatedUserData);
      const modifiedUserIndex = authUsers.value.findIndex(user => user.id === userId);
      if (modifiedUserIndex !== -1)
      {
        authUsers.value[modifiedUserIndex] =
        {
          ...authUsers.value[modifiedUserIndex],
          ...updatedUserData,
        };
      }
      closeModifyDialog();
    } 
    catch (error:any)
    {
      console.error('Error modifying user:', error.message);
    }
  };
  // Communicates with UserController service to get data from database and sort it in the needed order
  const getUsersAndSortData = async () => 
  {
    try 
    {
      authUsers.value = await UserController.getUsersAndSort(sortField.value, sortOrder.value);
    } 
    catch (error:any) 
    {
      console.error(error);
    }
  };

  onMounted(() =>
  {
    document.body.classList.remove('login-overflow');
    document.body.classList.add('regular-overflow');
    getUsersAndSortData();
    const currentRoute = router.currentRoute.value;
    // Checks if query contains log, if it does, then the page was redirected from logs and contains query contains data to add a new user from the log to the database
    if ('log' in currentRoute.query && currentRoute.query.log !== null) 
    {
      const decodedLog = decodeURIComponent(currentRoute.query.log as string);
      const logObj = JSON.parse(decodedLog);
      openAddDialog(logObj);
      router.push({ query: {} });
    }
  });
  // Selects all rows when checkbox is ticked
  const selectAllRows = () =>
  {
    if (selectAll.value)
    {
      selectedRows.value = authUsers.value.map((user) => user.id);
    }
    else
    {
      selectedRows.value = [];
    }
  };
  // Sorts the table when user clicks on a sortable header
  const sortTable = (field:any) =>
  {
    if (sortField.value === field)
    {
      sortOrder.value = -sortOrder.value; // Reverse order if sorting by the same field
    } 
    else
    {
      sortField.value = field;
      sortOrder.value = 1;
    }
    getUsersAndSortData();
  };
  // Deletes one or multiple rows that are selected with checkboxes
  const deleteSelectedRows = async () =>
  {
    for (const id of selectedRows.value)
    {
      try
      {
        await UserController.removeUser(id);
        authUsers.value = authUsers.value.filter((user) => user.id !== id);
      } 
      catch (error:any)
      {
        console.error('Error removing user:', error.message);
      }
    }
    // Clear the selected rows
    selectedRows.value = [];
    selectAll.value = false;
  };
  // Communicates with UserController service to remove the user from database after confirmation
  const removeUser = async (id:any) =>
  {
    try
    {
      await UserController.removeUser(id);
      authUsers.value = authUsers.value.filter((user) => user.id !== id);
    } 
    catch (error: any)
    {
      console.error('Error removing user:', error.message);
    }
  };
  // Calls the user removal and removes user from the table
  const handleRemoveUser = async (idToRemove:any) =>
  {
    await removeUser(idToRemove).catch(err => console.log(err.message));
    selectedRows.value = selectedRows.value.filter((id) => id !== idToRemove);
  };

</script>

<style scoped>

  .error
  {
    color: red;
    font-family: 'Satoshi-Regular';
    font-size: 0.8rem;
    margin-left: 6px;
  }

  .log-box
  {
    background-color: #101010;
    padding: 15px;
    border-radius: 8px;
    border-top-right-radius:0px;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    max-width: 100%;
    max-height: 100%;
    display: flex;
    flex-direction: column;
  }

  .flex-parent-element 
  {
    display: inline-flex;
    width: 100%;
  }

  .flex-child-element-title
  {
    flex:1;
    background-color: #101010;
    border-top-left-radius:8px;
    border-top-right-radius:8px;
    padding-top:10px;
  }

  .lbl-title
  {
    font-size: clamp(1rem, 1.5rem, 3rem);
    font-family: 'Satoshi-Black';
    white-space: nowrap;
  }
  
  .flex-child-element-button
  {
    text-align: left;
    flex:0;
  }

  .checkbox-column
  {
    text-align: left; 
    color: #fff;
    color-scheme:dark;
    width: fit-content;
  }
  .checkbox-container
  {
    width: 40px;
    align-items: center;
    display: inline-flex;
  }
  .delete-button
  {
    margin-left:20px;
    padding-block: 2px;
    padding-inline: 30px;
    border-radius: 6px;
    text-align: center;
    font-family: 'Satoshi-Medium';
    background-color: #00000000;
  }
  .delete-button:hover
  {
    background-color: #101010;
    color: #fff;
  }

  .selected-row
  {
    background-color: #242424;
  }
  </style>
