import { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import api from './api';
import Spinner from './components/Spinner';
import Button from './components/ui/Button';
import Input from './components/ui/Input';

export default function Login({ onLogin }: { onLogin: (token: string) => void }) {
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [show, setShow] = useState(false);
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);
  const navigate = useNavigate();
  return (
    <div className="flex flex-col items-center justify-center h-screen">
      <div className="bg-[#1a1a1a] p-6 rounded shadow-md w-80">
        <h1 className="text-2xl mb-4 text-center text-neonBlue">ULTSPY Login</h1>
        <Input
          placeholder="Benutzername"
          value={username}
          onChange={(e) => setUsername(e.target.value)}
          className="mb-4"
        />
        <div className="relative mb-4">
          <Input
            type={show ? 'text' : 'password'}
            className="pr-10"
            placeholder="Passwort"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
          />
          <button
            type="button"
            onClick={() => setShow(!show)}
            className="absolute right-2 top-2 text-gray-400 hover:text-gray-200"
          >
            {show ? '🙈' : '👁️'}
          </button>
        </div>
        <Button
          className="w-full"
          onClick={async () => {
            setLoading(true);
            setError('');
            try {
              const res = await api.post('/login', { username, password });
              localStorage.setItem('ULTSPY_JWT', res.data.token);
              onLogin(res.data.token);
              navigate('/');
            } catch {
              localStorage.removeItem('ULTSPY_JWT');
              setError('Ungültiger Login oder keine Verbindung zum Server.');
            } finally {
              setLoading(false);
            }
          }}
        >
          {loading ? <Spinner /> : 'Login'}
        </Button>
        {error && <p className="text-red-500 mt-2 text-center text-sm">{error}</p>}
      </div>
    </div>
  );
}
