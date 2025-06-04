import { useState } from 'react';
import { QueryClient, QueryClientProvider } from '@tanstack/react-query';
import {
  BrowserRouter,
  Routes,
  Route,
} from 'react-router-dom';
import Login from './Login';
import Dashboard from './Dashboard';
import AgentDetail from './features/agents/AgentDetail';

const queryClient = new QueryClient();

export default function App() {
  const [loggedIn, setLoggedIn] = useState(() => !!localStorage.getItem('apiKey'));

  return (
    <QueryClientProvider client={queryClient}>
      {loggedIn ? (
        <BrowserRouter>
          <Routes>
            <Route path="/" element={<Dashboard />} />
            <Route path="/agents/:uuid" element={<AgentDetail />} />
          </Routes>
        </BrowserRouter>
      ) : (
        <Login onLogin={() => setLoggedIn(true)} />
      )}
    </QueryClientProvider>
  );
}
