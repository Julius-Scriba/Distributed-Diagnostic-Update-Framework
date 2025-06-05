import { useQuery } from '@tanstack/react-query';
import api from '../api';

export interface Agent {
  uuid: string;
  online: boolean;
  last_seen: string;
}

interface AgentsResponse {
  agents: Agent[];
}

export default function useAgents() {
  return useQuery({
    queryKey: ['agents'],
    queryFn: async () => (await api.get<AgentsResponse>('/admin/agents')).data.agents,
  });
}
