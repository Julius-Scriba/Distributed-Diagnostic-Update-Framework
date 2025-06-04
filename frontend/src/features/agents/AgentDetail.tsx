import { useState } from 'react';
import { useParams } from 'react-router-dom';
import useAgents from '../../api/useAgents';
import useSendCommand from '../../api/useSendCommand';

export default function AgentDetail() {
  const { uuid } = useParams<{ uuid: string }>();
  const { data: agents, isLoading } = useAgents();
  const agent = agents?.find(a => a.uuid === uuid);

  const [command, setCommand] = useState('SAFE_MODE');
  const sendCommand = useSendCommand(uuid || '');

  if (isLoading) {
    return <div className="p-4 text-white">Loading...</div>;
  }

  if (!agent) {
    return <div className="p-4 text-white">Agent not found</div>;
  }

  return (
    <div className="p-4 text-white">
      <h1 className="text-2xl mb-4">Agent {agent.uuid}</h1>
      <div className="mb-4">
        <p>Status: {agent.online ? 'Online' : 'Offline'}</p>
        <p>Last Seen: {agent.last_seen}</p>
      </div>

      <div className="bg-gray-800 p-4 rounded w-80">
        <label className="block mb-2">Command</label>
        <select
          className="w-full bg-gray-700 p-2 mb-4 rounded"
          value={command}
          onChange={e => setCommand(e.target.value)}
        >
          <option value="SAFE_MODE">SAFE_MODE</option>
          <option value="WIPE">WIPE</option>
          <option value="DEEP_SLEEP">DEEP_SLEEP</option>
        </select>
        <button
          className="w-full bg-blue-600 hover:bg-blue-700 py-2 rounded"
          onClick={() => sendCommand.mutate({ command, parameters: {} })}
        >
          Send Command
        </button>
        {sendCommand.isSuccess && (
          <p className="text-green-400 mt-2 text-center">Command sent successfully</p>
        )}
        {sendCommand.isError && (
          <p className="text-red-400 mt-2 text-center">Error sending command</p>
        )}
      </div>
    </div>
  );
}
