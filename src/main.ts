import { createApp } from 'vue';
import router from '@/router';
import './style.css'; // GLobal style file
import './assets/css/satoshi.css'; //For satoshi font
import App from '@/App.vue';
import 'virtual:windi.css';
import PrimeVue from 'primevue/config'; // For ripple and icons
import { vMaska } from "maska"; // For input masking
import Ripple from 'primevue/ripple'; // For mobile buttons
import 'primevue/resources/primevue.min.css'; //core css
import 'primeicons/primeicons.css'; //icons

createApp(App)
    .use(router)
    .use(PrimeVue, { ripple: true })
    .directive("maska", vMaska)
    .directive('ripple', Ripple)
    .mount('#app');



