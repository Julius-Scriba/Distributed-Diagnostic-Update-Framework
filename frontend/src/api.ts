import axios from 'axios';

let handleUnauthorized: (() => void) | null = null;
let handleNetworkError: (() => void) | null = null;

export function setUnauthorizedHandler(fn: () => void) {
  handleUnauthorized = fn;
}

export function setNetworkErrorHandler(fn: () => void) {
  handleNetworkError = fn;
}

const api = axios.create({
  baseURL: '/api',
});

api.interceptors.request.use(config => {
  if (config.url?.startsWith('/admin')) {
    const token = localStorage.getItem('ULTSPY_JWT');
    if (token) {
      config.headers['Authorization'] = `Bearer ${token}`;
    }
  }
  return config;
});

api.interceptors.response.use(
  res => res,
  error => {
    if (error.response?.status === 401) {
      handleUnauthorized?.();
    } else if (!error.response) {
      handleNetworkError?.();
    }
    return Promise.reject(error);
  }
);

export default api;
