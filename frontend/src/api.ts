import axios from 'axios';

const api = axios.create({
  baseURL: '/api',
});

api.interceptors.request.use(config => {
  if (config.url?.startsWith('/admin')) {
    const apiKey = localStorage.getItem('ULTSPY_API_KEY');
    if (apiKey) {
      config.headers['X-API-KEY'] = apiKey;
    }
  }
  return config;
});

export default api;
