import { useState } from 'react';
import { QueryClient, QueryClientProvider } from '@tanstack/react-query';
import Login from './Login';
import Dashboard from './Dashboard';

const queryClient = new QueryClient();

export default function App() {
  const [loggedIn, setLoggedIn] = useState(() => !!localStorage.getItem('apiKey'));

  return (
    <QueryClientProvider client={queryClient}>
      {loggedIn ? <Dashboard /> : <Login onLogin={() => setLoggedIn(true)} />}
    </QueryClientProvider>
  );
}
