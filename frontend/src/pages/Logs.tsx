import { useState } from 'react';
import useAgents from '../api/useAgents';
import useLogs from '../api/useLogs';

export default function Logs() {
  const { data: agents } = useAgents();
  const [selected, setSelected] = useState('');
  const { data: logs, isLoading, isError } = useLogs(selected || null);

  return (
    <div className="p-4 text-white space-y-4">
      <h1 className="text-2xl mb-2">Logs</h1>
      <div>
        <label className="mr-2">Select Agent:</label>
        <select
          className="bg-gray-700 p-2 rounded"
          value={selected}
          onChange={(e) => setSelected(e.target.value)}
        >
          <option value="">-- choose --</option>
          {agents?.map((a) => (
            <option key={a.uuid} value={a.uuid}>
              {a.uuid}
            </option>
          ))}
        </select>
      </div>

      {selected && (
        <>
          {isLoading && <p>Loading...</p>}
          {isError && (
            <p className="text-red-400">Failed to load logs</p>
          )}
          {logs && (
            <table className="min-w-full bg-gray-800">
              <thead>
                <tr>
                  <th className="py-2">Timestamp</th>
                  <th>Type</th>
                  <th>Description</th>
                  <th>Details</th>
                </tr>
              </thead>
              <tbody>
                {logs.map((log, idx) => (
                  <tr key={idx} className="text-center">
                    <td className="py-1 px-2">{log.timestamp}</td>
                    <td>{log.type}</td>
                    <td>{log.description}</td>
                    <td>
                      {log.data && (
                        <details>
                          <summary className="cursor-pointer text-blue-400">
                            View
                          </summary>
                          <pre className="bg-gray-700 p-2 mt-1 text-left whitespace-pre-wrap">
                            {log.data}
                          </pre>
                        </details>
                      )}
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          )}
        </>
      )}
    </div>
  );
}
