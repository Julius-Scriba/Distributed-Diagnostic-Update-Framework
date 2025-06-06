import { useState, useEffect } from 'react';
import { QueryClient, QueryClientProvider } from '@tanstack/react-query';
import {
  BrowserRouter,
  Routes,
  Route,
  Navigate,
  Outlet,
} from 'react-router-dom';
import Login from './Login';
import Layout from './layout/Layout';
import { setUnauthorizedHandler, setNetworkErrorHandler } from './api';
import Dashboard from './pages/Dashboard';
import Agents from './pages/Agents';
import Commands from './pages/Commands';
import Logs from './pages/Logs';
import Settings from './pages/Settings';
import Templates from './pages/Templates';
import AgentDetail from './features/agents/AgentDetail';

const queryClient = new QueryClient();

function RequireAuth({ loggedIn }: { loggedIn: boolean }) {
  return loggedIn ? <Outlet /> : <Navigate to="/login" replace />;
}

export default function App() {
  const [loggedIn, setLoggedIn] = useState(() => !!localStorage.getItem('ULTSPY_API_KEY'));
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    setUnauthorizedHandler(() => {
      localStorage.removeItem('ULTSPY_API_KEY');
      setLoggedIn(false);
      window.location.href = '/login';
    });
    setNetworkErrorHandler(() => setError('Verbindung zum Backend unterbrochen.'));
  }, []);

  const handleLogout = () => {
    localStorage.removeItem('ULTSPY_API_KEY');
    setLoggedIn(false);
  };

  return (
    <QueryClientProvider client={queryClient}>
      <BrowserRouter>
        <Routes>
          <Route path="/login" element={<Login onLogin={() => setLoggedIn(true)} />} />
          <Route element={<RequireAuth loggedIn={loggedIn} />}>
            <Route element={<Layout onLogout={handleLogout} error={error} clearError={() => setError(null)} />}>
              <Route path="/" element={<Dashboard />} />
              <Route path="/agents" element={<Agents />} />
              <Route path="/commands" element={<Commands />} />
              <Route path="/templates" element={<Templates />} />
              <Route path="/logs" element={<Logs />} />
              <Route path="/settings" element={<Settings />} />
              <Route path="/agents/:uuid" element={<AgentDetail />} />
            </Route>
          </Route>
        </Routes>
      </BrowserRouter>
    </QueryClientProvider>
  );
}
