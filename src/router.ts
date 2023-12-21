import { RouteRecordRaw, createRouter, createWebHistory } from 'vue-router'
import Home from '@/views/Home.vue'
import ViewLog from '@/views/ViewLog.vue'
import StudentList from '@/views/StudentList.vue'
import ErrorPage from '@/views/ErrorPage.vue'
import Settings from '@/views/Settings.vue'
import Login from './views/Login.vue'
import firebaseInstance from './backend/firebase'
import { getAuth } from 'firebase/auth'

const routes: Array<RouteRecordRaw> =[
    {
      path: '/',
      name: 'Home',
      component: Home,
      props: true,
      meta: {
        title: 'Tark laborikapp',
        requiresAuth: true,
      },
    },
    {
      path: '/logs',
      name: 'Logs',
      component: ViewLog,
      props: { title: 'Logid'},
      meta: {
        title: 'Logi',
        requiresAuth: true,
      },
    },
    {
      path: '/users',
      name: 'List of users',
      component: StudentList,
      props: true,
      meta: {
        title: 'Autoriseeritud kasutajad',
        requiresAuth: true,
      },
    },
    {
      path: '/settings',
      name: 'Locker settings',
      component: Settings,
      props: true,
      meta: 
      {
        title: 'Kapi seaded',
        requiresAuth: true,
      },
    },
    {
      path: '/login',
      name: 'Login',
      component: Login,
      props: true,
      meta: 
      {
        showNavBar: false,
      },
    },
    {
      path: '/:catchAll(.*)', // Teadmata pathide püüdmiseks
      name: 'NotFound',
      component: ErrorPage, // Display the error page component
      meta: {
        title: 'Lehte ei leitud',
        requiresAuth: true,
      },
    },
  ];

const router = createRouter(
{
    history: createWebHistory(),
    routes,
});

router.beforeEach(async (to, _, next) => {
  const auth = getAuth(firebaseInstance);
  const user = await new Promise((resolve) => {
    const unsubscribe = auth.onAuthStateChanged((user) => {
      unsubscribe();
      resolve(user);
    });
  });

  if (to.meta.requiresAuth && !user) {
    next('/login');
  } else if (to.name === 'Login' && user) {
    // If the user is already authenticated and tries to access the login page, redirect to the home page
    next('/');
  } else {
    next();
  }

  if (to.meta.title) {
    document.title = to.meta.title as string;
  } else {
    document.title = 'Tark laborikapp';
  }
}); 
  
export default router;
