import { useState } from 'react';
import { useApiKeys, useCreateApiKey, useRevokeApiKey } from '../api/useApiKeys';
import Spinner from '../components/Spinner';
import { formatDistanceToNow } from 'date-fns';

export default function ApiKeys() {
  const { data, isLoading, error } = useApiKeys();
  const create = useCreateApiKey();
  const revoke = useRevokeApiKey();
  const [name, setName] = useState('');
  const [newSecret, setNewSecret] = useState<string | null>(null);

  if (isLoading) return <Spinner />;
  if (error) return <div className="text-red-600">Error</div>;

  return (
    <div className="space-y-4">
      <h1 className="text-xl font-bold">API Keys</h1>
      <form
        onSubmit={e => {
          e.preventDefault();
          create.mutate(name, {
            onSuccess: d => {
              setNewSecret(d.secret);
              setName('');
            },
          });
        }}
        className="space-x-2"
      >
        <input
          value={name}
          onChange={e => setName(e.target.value)}
          placeholder="Name"
          className="border px-2 py-1"
        />
        <button className="bg-blue-600 text-white px-3 py-1 rounded" type="submit">
          Create
        </button>
      </form>
      {newSecret && (
        <div className="bg-yellow-100 p-2 border">New key: {newSecret}</div>
      )}
      <table className="min-w-full bg-white">
        <thead>
          <tr>
            <th className="text-left p-2">Name</th>
            <th className="text-left p-2">Created</th>
            <th className="text-left p-2">Last Used</th>
            <th></th>
          </tr>
        </thead>
        <tbody>
          {data?.map(k => (
            <tr key={k.id} className="border-t">
              <td className="p-2">{k.name}</td>
              <td className="p-2">{formatDistanceToNow(new Date(k.created_at), { addSuffix: true })}</td>
              <td className="p-2">
                {k.last_used_at ? formatDistanceToNow(new Date(k.last_used_at), { addSuffix: true }) : 'never'}
              </td>
              <td className="p-2 text-right">
                <button
                  onClick={() => revoke.mutate(k.id)}
                  className="text-red-600 underline"
                >
                  Revoke
                </button>
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}
