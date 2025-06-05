import useConfig from '../api/useConfig';

export default function Settings() {
  const { data, isLoading, isError } = useConfig();

  return (
    <div className="p-4 text-white space-y-4">
      <h1 className="text-2xl mb-2">Settings</h1>
      {isLoading && <p>Loading...</p>}
      {isError && <p className="text-red-400">Failed to load settings</p>}
      {data && (
        <div className="space-y-4">
          <div>
            <h2 className="text-xl mb-1">Version</h2>
            <p>Backend: {data.versions.backend}</p>
            <p>Frontend: {data.versions.frontend}</p>
          </div>
          <table className="min-w-full bg-gray-800">
            <tbody>
              <tr>
                <td className="py-1 px-2">Heartbeat Timeout</td>
                <td>{data.heartbeat_timeout} seconds</td>
              </tr>
              <tr>
                <td className="py-1 px-2">Allowed Hosts</td>
                <td>{data.allowed_hosts.join(', ') || '-'}</td>
              </tr>
              <tr>
                <td className="py-1 px-2">Targets</td>
                <td>{data.targets.join(', ') || '-'}</td>
              </tr>
              <tr>
                <td className="py-1 px-2">API Keys</td>
                <td>{data.api_keys.join(', ')}</td>
              </tr>
            </tbody>
          </table>
        </div>
      )}
    </div>
  );
}
