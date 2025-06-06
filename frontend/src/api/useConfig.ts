import { useQuery } from '@tanstack/react-query';
import api from '../api';

export interface ConfigData {
  api_keys: string[];
  heartbeat_timeout: number;
  versions: {
    backend: string;
    frontend: string;
  };
  targets: string[];
  allowed_hosts: string[];
}

export default function useConfig() {
  return useQuery({
    queryKey: ['config'],
    queryFn: async () => {
      const { data } = await api.get<ConfigData>('/admin/config');
      return data;
    },
  });
}
