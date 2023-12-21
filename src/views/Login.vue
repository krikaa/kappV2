<template>
    <div style="height:100%" id="login-page">
        <div v-if="loading"><i class="pi pi-spin pi-spinner" style="font-size: 4rem"></i>
            <br>
            <div style="font-family: 'Satoshi-Medium'">Laen...</div>
        </div>
        <div v-else>
            <div class="background-zoom"></div>
            <div style="max-width:90vw" class="login-box" >
                <h1 class="login-header">Sisselogimine</h1>
                <form @submit.prevent="login" style="width: 100%">
                    <div class="login-field" style="text-align:left">
                        <div>
                            <label class="label-input">Kasutajanimi</label>
                            <input class="login-input" :class="{ 'error-input': v$.uname.$error}" placeholder="Sisestage kasutajanimi" v-model="uname" autocomplete="nickname">
                            <div v-if="v$.uname.$error" class="errorName" title="See väli on kohustuslik"><i class="pi pi-info-circle" style="font-size: 0.9rem"></i></div>
                        </div>
                    </div>
                    <div class="login-field" style="text-align:left">
                        <div>
                            <label class="label-input">Parool</label>
                            <input class="login-input" :class="{ 'error-input': v$.passwd.$error}" type="password" placeholder="Sisestage parool" v-model="passwd" autocomplete="current-password">
                            <div v-if="v$.passwd.$error" class="errorPass" title="See väli on kohustuslik"><i class="pi pi-info-circle" style="font-size: 0.9rem"></i>
                            </div>
                        </div>
                    </div>
                    <div class="errorMsg" v-if="errorMessage">
                        <i class="pi pi-times-circle" style="font-size: 0.8rem"></i> {{ errorMessage }}
                    </div>
                    <div>
                        <button v-ripple="{ pt: { root: { style: 'background: rgba(57, 172, 231, 0.9)' }}}" value="login" class="p-ripple dialog-button" style="margin-top: 30px">Logi sisse</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
    import { ref, onBeforeMount } from 'vue';
    import firebaseInstance from '@/backend/firebase';
    import { getAuth, signInWithEmailAndPassword } from 'firebase/auth';
    import { useVuelidate } from '@vuelidate/core';
    import { required } from '@vuelidate/validators'

    const uname = ref("");
    const passwd = ref("");
    const loading = ref(true);
    const errorMessage = ref('');

    const rules = 
    {
        uname: { required },
        passwd: { required }
    };
    // vuelidate object to validate the rules
    const v$ = useVuelidate(rules, 
    {
        uname,
        passwd
    });
    // Authenticates the user
    const login = () =>
    {
        if (v$.value.$invalid)
        {
            v$.value.$touch();
            return;
        }
        signInWithEmailAndPassword(getAuth(firebaseInstance), uname.value + "@nutikapp.ee", passwd.value)
            .then(data => console.log(data))
            .catch(err => handleLoginError(err));
    };
    // Modifies error text value in Estonian based on the error message
    const handleLoginError = (error:any) =>
    {
        console.log(error.code);
        switch (error.code)
        {
            case 'auth/invalid-login-credentials':
                errorMessage.value = 'Vale kasutajanimi või parool. Proovi uuesti.';
                break;

            case 'auth/too-many-requests':
                errorMessage.value = 'Liiga palju katseid. Proovi hiljem uuesti.';
                break;
            
            default:
                errorMessage.value = 'Viga sisselogimisel. Palun kontrolli andmeid.'
        }
    };

    onBeforeMount(() =>
    {
        setTimeout(() =>
        {
            loading.value = false;
        }, 600);
        // Changing the class of document body to apply overflow for login page on mobile devices
        document.body.classList.remove('regular-overflow');
        document.body.classList.add('login-overflow');
    });

</script>

<style scoped>
    .login-field
    {
        position: relative;
    }

    .login-input.error-input
    {
        border: 1px solid red; 
    }

    .errorMsg
    {
        font-family: 'Satoshi-Regular';
        font-size: 0.9rem;
        color: red;
        position:absolute;
    }

    .errorName, .errorPass
    {
        color: red;
        font-family: 'Satoshi-Regular';
        font-size: 0.8rem;
        position: absolute;
        right: 3%;
        top: 63%;
        transform: translateY(-50%);
    }

    /* Media query for mobile use */
    @media (max-width: 768px)
    {
        .errorName, .errorPass {
            top: 63%;
            right: 3%;
            transform: translateY(-50%);
        }

        .errorMsg {
            position: relative;
        }
    }

</style>
