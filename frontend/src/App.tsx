import { useState } from 'react';
import { QueryClient, QueryClientProvider } from '@tanstack/react-query';
import {
  BrowserRouter,
  Routes,
  Route,
} from 'react-router-dom';
import Login from './Login';
import Layout from './layout/Layout';
import Dashboard from './pages/Dashboard';
import Agents from './pages/Agents';
import Commands from './pages/Commands';
import Logs from './pages/Logs';
import Settings from './pages/Settings';
import AgentDetail from './features/agents/AgentDetail';

const queryClient = new QueryClient();

export default function App() {
  const [loggedIn, setLoggedIn] = useState(() => !!localStorage.getItem('apiKey'));

  return (
    <QueryClientProvider client={queryClient}>
      {loggedIn ? (
        <BrowserRouter>
          <Routes>
            <Route element={<Layout />}>
              <Route path="/" element={<Dashboard />} />
              <Route path="/agents" element={<Agents />} />
              <Route path="/commands" element={<Commands />} />
              <Route path="/logs" element={<Logs />} />
              <Route path="/settings" element={<Settings />} />
              <Route path="/agents/:uuid" element={<AgentDetail />} />
            </Route>
          </Routes>
        </BrowserRouter>
      ) : (
        <Login onLogin={() => setLoggedIn(true)} />
      )}
    </QueryClientProvider>
  );
}
