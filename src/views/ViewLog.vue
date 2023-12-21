<template>
  <div id="log-page">
    <div class="log-box">
      <h2>Logi</h2>
      <div class="table-wrapper scrollbar">
        <table class="logs-table">
          <thead class="fixed-header">
            <tr>
              <th title="Sissekande number">#</th>
              <th class="sortable" title="Sorteeri: kuupäev" @click="sortTable('timestamp')">Kuupäev ja kellaaeg</th>
              <th class="sortable" title="Sorteeri: sündmus" @click="sortTable('Event')">Sündmus</th>
              <th class="sortable" title="Sorteeri: nimi" @click="sortTable('name')">
                Nimi   <i v-if="isOwnerSorted === 1" class="pi pi-sort-alpha-down"></i>
                <i v-else-if="isOwnerSorted === -1" class="pi pi-sort-alpha-down-alt"></i>
              </th>
              <th title="Sorteeri: tüüp" class="sortable" @click="sortTable('type')">
                Tüüp  <i v-if="isTypeSorted === 1" class="pi pi-sort-alpha-down"></i>
                <i v-else-if="isTypeSorted === -1" class="pi pi-sort-alpha-down-alt"></i>
              </th>
              <th>UUID</th>
              <th></th>
            </tr>
          </thead>
          <tbody v-if="logsData.length > 0" class="table-body">
            <tr v-for="(log, idx) in logsData" :key="log.timestamp" class="logs-row">
              <td>{{ idx + 1 }}</td>
              <td>{{ formatTimestamp(log.timestamp) }}</td>
              <td>{{ log.Event }}</td>
              <td>{{ log.name }}</td>
              <td>{{ log.type }}</td>
              <td>{{ log.UUID }}</td>
              <td>
                <div v-if="showAddButton(log)">
                  <button @click="openAddDialog(log)" id="add-button" title="Lisa kasutajaks"><i class="pi pi-user-plus" style="font-size: 1.1rem;"></i>
                  </button>
                </div>
                <div v-else></div>
              </td>
            </tr>
          </tbody>
          <tbody v-else>
            <br>
            <tr>
              <td colspan="6">Logi sissekanded hetkel puuduvad.</td>
            </tr>
          </tbody>
        </table>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
  import { ref, onMounted, onBeforeUnmount, computed } from 'vue';
  import LogController from '@/components/LogController'; 
  import router from '@/router'; 
  // Format for a log JSON object
  const logsData = ref<
  {
    Event: string;
    UUID: string;
    timestamp: number;
    name: string;
    type: string;
    uuidExists: boolean;
  }[]>([]);
  const sortField = ref('timestamp');
  const sortOrder = ref(-1);
  const isOwnerSorted = computed(() => sortField.value === 'name' ? sortOrder.value : 0);
  const isTypeSorted = computed(() => sortField.value === 'type' ? sortOrder.value : 0);
  // Shows add user button on log entry, if it doesnt already exist in users database
  const showAddButton = (log: any) =>
  {
    const isUnknownType = log.type === 'unknown';
    const hasOtherTypes = logsData.value.some(entry => entry.UUID === log.UUID && entry.type !== 'unknown');
    return isUnknownType && !hasOtherTypes && !log.uuidExists;
  };

  // Adds selected log as a query parameter and uses router to navigate to users page to add the user
  const openAddDialog = (log:any) =>
  {
    const serializedLog = JSON.stringify(log);
    router.push(
    {
      path: '/users',
      query:
      {
        log: encodeURIComponent(serializedLog),
      },
    });
  }; 
  // Communicates with log controller to get logs from database and sort them
  const getLogsAndSortData = async () => 
  {
    try 
    {
      logsData.value = await LogController.getLogsAndSort(sortField.value, sortOrder.value);
    } 
    catch (error:any) 
    {
      console.error(error);
    }
  };
  // Sorts the table based on clicked header
  const sortTable = (field:any) => 
  {
    sortField.value = field;
    sortOrder.value = sortOrder.value === 1 ? -1 : 1;
    getLogsAndSortData();
  };
  // Formats linux epoch timestamp to local readable format
  const formatTimestamp = (timestamp: number) => 
  {
    const date = new Date(timestamp);
    const options: Intl.DateTimeFormatOptions = 
    {
      year: 'numeric',
      month: '2-digit',
      day: '2-digit',
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit',
    };
    return date.toLocaleString('et-EE', options);
  };

  onMounted(() => 
  {
    document.body.classList.remove('login-overflow');
    document.body.classList.add('regular-overflow');
    getLogsAndSortData();
    // To refresh the logs every 5 seconds
    const intervalId = setInterval(() =>
    {
      //console.log('Refreshing table data...');
      getLogsAndSortData();
    }, 5000);

    onBeforeUnmount(() =>
    {
      clearInterval(intervalId);
    });
  });
  
</script>

<style scoped>
  h2 {
    background-color: #101010;
    font-family: 'Satoshi-Black';
    font-size: clamp(1rem, 1.5rem, 3rem);
    margin-bottom: 10px;
    margin-top: 0;
  }

  .log-box {
    background-color: #101010;
    padding: 15px;
    border-radius: 8px;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    max-width: 100%;
    max-height: 100%;
    display: flex !important;
    flex-direction: column; 
  }

</style>
